
#include "dicomsender.h"
#include "sqlite3_exec_stmt.h"
#include <boost/lexical_cast.hpp>
#include <codecvt>

// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"

// check DCMTK functionality
#if !defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
#error "DCMTK and this program must be compiled with DCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS"
#endif

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif

class DICOMSenderImpl
{	

public:
	DICOMSenderImpl(void);
	~DICOMSenderImpl(void);

	void Initialize(sqlite3 *db, const std::string PatientName, std::string PatientID ,std::string BirthDay,
		std::string NewPatientName, std::string NewPatientID ,std::string NewBirthDay,
		std::string destinationHost, unsigned int destinationPort, std::string destinationAETitle, std::string ourAETitle);	

	static void DoSendThread(void *obj);

	std::string ReadLog();

	void Cancel();
	bool IsDone();

	sqlite3 *db;

	std::string m_PatientName;
	std::string m_PatientID;
	std::string m_BirthDay;
	std::string m_NewPatientName;
	std::string m_NewPatientID;
	std::string m_NewBirthDay;

	std::string m_destinationHost;
	unsigned int m_destinationPort;
	std::string m_destinationAETitle;
	std::string m_ourAETitle;

protected:
	static int countcallback(void *param,int columns,char** values, char**names);

	void DoSend();

	int SendABatch();

	bool IsCanceled();
	void SetDone(bool state);

	boost::mutex mutex;
	bool cancelEvent, doneEvent;

	class GUILog
	{
	public:
		void Write(const char *msg);
		void Write(const OFCondition &cond);
		void Write(std::stringstream &stream);
		std::string Read();
	protected:
		std::stringstream mycout;	
		boost::mutex mycoutmutex;
	};

	GUILog log;

	OFCondition storeSCU(T_ASC_Association * assoc, const boost::filesystem::path &fname);	
	void replacePatientInfoInformation(DcmDataset* dataset);
	// void replaceSOPInstanceInformation(DcmDataset* dataset);
	OFCondition addStoragePresentationContexts(T_ASC_Parameters *params, OFList<OFString>& sopClasses);	
	OFString makeUID(OFString basePrefix, int counter);
	bool updateStringAttributeValue(DcmItem* dataset, const DcmTagKey& key, OFString value);

	/*  glue functions to transfer our db to the list*/
	void addfiles();
	static int addimage(void *param,int columns,char** values, char**names);

	std::vector<boost::filesystem::path> fileNameList;
	OFList<OFString> sopClassUIDList;    // the list of sop classes



	OFCmdUnsignedInt opt_maxReceivePDULength;
	OFCmdUnsignedInt opt_maxSendPDULength;
	E_TransferSyntax opt_networkTransferSyntax;

	
	bool opt_combineProposedTransferSyntaxes;

	bool opt_correctUIDPadding;
	OFString patientNamePrefix;   // PatientName is PN (maximum 16 chars)
	OFString patientIDPrefix; // PatientID is LO (maximum 64 chars)
	OFString studyIDPrefix;   // StudyID is SH (maximum 16 chars)
	OFString accessionNumberPrefix;  // AccessionNumber is SH (maximum 16 chars)
	bool opt_secureConnection; /* default: no secure connection */		
	T_DIMSE_BlockingMode opt_blockMode;
	int opt_dimse_timeout;	
	int opt_timeout;

	OFCmdUnsignedInt opt_compressionLevel;		

	static void progressCallback(void * callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ * req);
};

DICOMSenderImpl::DICOMSenderImpl()
{			
	opt_maxSendPDULength = 0;
	opt_networkTransferSyntax = EXS_Unknown;
	// opt_networkTransferSyntax = EXS_JPEG2000LosslessOnly;

	
	opt_combineProposedTransferSyntaxes = true;

	patientNamePrefix = "OFFIS^TEST_PN_";   // PatientName is PN (maximum 16 chars)
	patientIDPrefix = "PID_"; // PatientID is LO (maximum 64 chars)
	studyIDPrefix = "SID_";   // StudyID is SH (maximum 16 chars)	

	opt_timeout = 10;

	opt_compressionLevel = 0;
}

DICOMSenderImpl::~DICOMSenderImpl()
{
}

void DICOMSenderImpl::Initialize(sqlite3 *db, const std::string PatientName, std::string PatientID ,std::string BirthDay,
		std::string NewPatientName, std::string NewPatientID ,std::string NewBirthDay,
		std::string destinationHost, unsigned int destinationPort, std::string destinationAETitle, std::string ourAETitle)
{
	cancelEvent = doneEvent = false;
	this->db = db;
	this->m_PatientName = PatientName;
	this->m_PatientID = PatientID;
	this->m_BirthDay = BirthDay;
	this->m_NewPatientName = NewPatientName;
	this->m_NewPatientID = NewPatientID;
	this->m_NewBirthDay = NewBirthDay;

	this->m_destinationHost = destinationHost;
	this->m_destinationPort = destinationPort;
	this->m_destinationAETitle = destinationAETitle;
	this->m_ourAETitle = ourAETitle;
}

void DICOMSenderImpl::DoSendThread(void *obj)
{
	DICOMSenderImpl *me = (DICOMSenderImpl *) obj;
	if (me)
	{
		me->SetDone(false);
		me->DoSend();
		me->SetDone(true);
	}

}

std::string DICOMSenderImpl::ReadLog()
{
	return log.Read();
}

void DICOMSenderImpl::GUILog::Write(const char *msg)
{
	boost::mutex::scoped_lock lk(mycoutmutex);
	mycout << msg;
}

void DICOMSenderImpl::GUILog::Write(const OFCondition &cond)
{
	OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); dumpmsg.append("\n");
	Write(dumpmsg.c_str());
}

void DICOMSenderImpl::GUILog::Write(std::stringstream &stream)
{	
	Write(stream.str().c_str());
	stream.str(std::string());
}

std::string DICOMSenderImpl::GUILog::Read()
{
	boost::mutex::scoped_lock lk(mycoutmutex);	
	std::string str = mycout.str();
	mycout.str("");
	return str;
}
void DICOMSenderImpl::DoSend()
{
	OFLog::configure(OFLogger::OFF_LOG_LEVEL);

	int retry = 0;
	int countbefore = 0;
	int countafter = 0;
	do
	{
		// get number of unsent images
		std::string anymoresql = "SELECT COUNT(*) FROM images WHERE name = ? AND sent = '0'";
		sqlite3_stmt *select;
		sqlite3_prepare_v2(db, anymoresql.c_str(), anymoresql.length(), &select, NULL);
		sqlite3_bind_text(select, 1, m_PatientName.c_str(), m_PatientName.length(), SQLITE_STATIC);
		sqlite3_exec_stmt(select, &countcallback, &countbefore, NULL);

		// batch send
		if (countbefore > 0)
			SendABatch();		

		sqlite3_exec_stmt(select, &countcallback, &countafter, NULL);
		sqlite3_finalize(select);

		// only do a sleep if there's more to send, we didn't send anything out, and we still want to retry
		if (countafter > 0 && countbefore == countafter && retry < 10000)
		{
			retry++;
			log.Write("Waiting 5 mins before retry\n");

			// sleep loop with cancel check, 5 minutes
			int sleeploop = 5 * 60 * 5;
			while (sleeploop > 0)
			{
#ifdef _WIN32
				Sleep(200);
#else
                usleep(200);
#endif
                sleeploop--;
				if (IsCanceled())
					break;
			}
		}
		else		// otherwise, the next loop is not a retry
		{
			retry = 0;
		}
	}
	while (!IsCanceled() && countafter > 0 && retry < 10000);


	// reset
	std::string updatesql = "UPDATE images SET sent = 0";
	sqlite3_exec(db, updatesql.c_str(), NULL, NULL, NULL);    
}

int DICOMSenderImpl::SendABatch()
{
	T_ASC_Network *net = NULL;
	T_ASC_Parameters *params = NULL;
	T_ASC_Association *assoc = NULL;

	try
	{
		// init network
		int acse_timeout = 20;
		OFCondition cond = ASC_initializeNetwork(NET_REQUESTOR, 0, acse_timeout, &net);
		if (cond.bad())
		{		
			log.Write(cond);
			throw std::runtime_error("ASC_initializeNetwork");
		}

		cond = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
		if (cond.bad())
		{		
			log.Write(cond);
			throw std::runtime_error("ASC_createAssociationParameters");
		}

		ASC_setAPTitles(params, m_ourAETitle.c_str(), m_destinationAETitle.c_str(), NULL);	

		DIC_NODENAME localHost;
		gethostname(localHost, sizeof(localHost) - 1);
		std::stringstream peerHost;
		peerHost << m_destinationHost << ":" << m_destinationPort;
		ASC_setPresentationAddresses(params, localHost, peerHost.str().c_str());

		log.Write("Loading files...\n");
		addfiles();
		cond = addStoragePresentationContexts(params, sopClassUIDList);

		if (cond.bad())
		{
			log.Write(cond);
			throw std::runtime_error("addStoragePresentationContexts");
		}

		log.Write("Requesting Association\n");

		cond = ASC_requestAssociation(net, params, &assoc);
		if (cond.bad())
		{
			if (cond == DUL_ASSOCIATIONREJECTED)
			{				
				T_ASC_RejectParameters rej;
				ASC_getRejectParameters(params, &rej);
				std::stringstream msg;
				msg << "Association Rejected:\n";
				ASC_printRejectParameters(msg, &rej);
				log.Write(msg);
				throw std::runtime_error("ASC_requestAssociation");
			}
			else
			{
				log.Write("Association Request Failed:\n");			
				log.Write(cond);
				throw std::runtime_error("ASC_requestAssociation");
			}
		}

		// display the presentation contexts which have been accepted/refused	
		std::stringstream msg;
		msg << "Association Parameters Negotiated:\n";
		ASC_dumpParameters(params, msg);
		log.Write(msg);

		/* count the presentation contexts which have been accepted by the SCP */
		/* If there are none, finish the execution */
		if (ASC_countAcceptedPresentationContexts(params) == 0)
		{
			log.Write("No Acceptable Presentation Contexts\n");
			throw new std::runtime_error("ASC_countAcceptedPresentationContexts");
		}

		/* do the real work, i.e. for all files which were specified in the */
		/* command line, transmit the encapsulated DICOM objects to the SCP. */
		cond = EC_Normal;

		for(std::vector<boost::filesystem::path>::iterator iter = fileNameList.begin(); iter != fileNameList.end(); iter++)		
		{
			if (IsCanceled())
				break;
			cond = storeSCU(assoc, *iter);			
		}

		fileNameList.clear();

		/* tear down association, i.e. terminate network connection to SCP */
		if (cond == EC_Normal)
		{
			log.Write("Releasing Association\n");

			cond = ASC_releaseAssociation(assoc);
			if (cond.bad())
			{			
				log.Write("Association Release Failed:\n");
				log.Write(cond);			
			}
		}
		else if (cond == DUL_PEERREQUESTEDRELEASE)
		{
			log.Write("Protocol Error: peer requested release (Aborting)\n");		
			ASC_abortAssociation(assoc);		
		}
		else if (cond == DUL_PEERABORTEDASSOCIATION)
		{
			log.Write("Peer Aborted Association\n");		
		}
		else
		{
			log.Write("SCU Failed:\n");
			log.Write(cond);		
			log.Write("Aborting Association\n");

			ASC_abortAssociation(assoc);		
		}
	}
	catch(...)
	{

	}

	if(assoc)
		ASC_destroyAssociation(&assoc);	
	
	if(net)
		ASC_dropNetwork(&net);


	return 0;
}


static bool
	isaListMember(OFList<OFString>& lst, OFString& s)
{	
	bool found = false;

	for(OFListIterator(OFString) itr = lst.begin(); itr != lst.end() && !found; itr++)
	{
		found = (s == *itr);	
	}

	return found;
}

static OFCondition
	addPresentationContext(T_ASC_Parameters *params,
	int presentationContextId, const OFString& abstractSyntax,
	const OFString& transferSyntax,
	T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT)
{
	const char* c_p = transferSyntax.c_str();
	OFCondition cond = ASC_addPresentationContext(params, presentationContextId,
		abstractSyntax.c_str(), &c_p, 1, proposedRole);
	return cond;
}

static OFCondition
	addPresentationContext(T_ASC_Parameters *params,
	int presentationContextId, const OFString& abstractSyntax,
	const OFList<OFString>& transferSyntaxList,
	T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT)
{
	// create an array of supported/possible transfer syntaxes
	const char** transferSyntaxes = new const char*[transferSyntaxList.size()];
	int transferSyntaxCount = 0;
	OFListConstIterator(OFString) s_cur = transferSyntaxList.begin();
	OFListConstIterator(OFString) s_end = transferSyntaxList.end();
	while (s_cur != s_end)
	{
		transferSyntaxes[transferSyntaxCount++] = (*s_cur).c_str();
		++s_cur;
	}

	OFCondition cond = ASC_addPresentationContext(params, presentationContextId,
		abstractSyntax.c_str(), transferSyntaxes, transferSyntaxCount, proposedRole);

	delete[] transferSyntaxes;
	return cond;
}

OFCondition DICOMSenderImpl::addStoragePresentationContexts(T_ASC_Parameters *params, OFList<OFString>& sopClasses)
{
	/*
	* Each SOP Class will be proposed in two presentation contexts (unless
	* the opt_combineProposedTransferSyntaxes global variable is true).
	* The command line specified a preferred transfer syntax to use.
	* This prefered transfer syntax will be proposed in one
	* presentation context and a set of alternative (fallback) transfer
	* syntaxes will be proposed in a different presentation context.
	*
	* Generally, we prefer to use Explicitly encoded transfer syntaxes
	* and if running on a Little Endian machine we prefer
	* LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
	* Some SCP implementations will just select the first transfer
	* syntax they support (this is not part of the standard) so
	* organise the proposed transfer syntaxes to take advantage
	* of such behaviour.
	*/

	// Which transfer syntax was preferred on the command line
	OFString preferredTransferSyntax;
	if (opt_networkTransferSyntax == EXS_Unknown)
	{
		/* gLocalByteOrder is defined in dcxfer.h */
		if (gLocalByteOrder == EBO_LittleEndian)
		{
			/* we are on a little endian machine */
			preferredTransferSyntax = UID_LittleEndianExplicitTransferSyntax;
		}
		else
		{
			/* we are on a big endian machine */
			preferredTransferSyntax = UID_BigEndianExplicitTransferSyntax;
		}
	}
	else
	{
		DcmXfer xfer(opt_networkTransferSyntax);
		preferredTransferSyntax = xfer.getXferID();
	}

	OFListIterator(OFString) s_cur;
	OFListIterator(OFString) s_end;


	OFList<OFString> fallbackSyntaxes;	
	fallbackSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
	fallbackSyntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
	fallbackSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);	

	// Remove the preferred syntax from the fallback list
	fallbackSyntaxes.remove(preferredTransferSyntax);
	// If little endian implicit is preferred then we don't need any fallback syntaxes
	// because it is the default transfer syntax and all applications must support it.
	if (opt_networkTransferSyntax == EXS_LittleEndianImplicit)
	{
		fallbackSyntaxes.clear();
	}

	// created a list of transfer syntaxes combined from the preferred and fallback syntaxes
	OFList<OFString> combinedSyntaxes;
	s_cur = fallbackSyntaxes.begin();
	s_end = fallbackSyntaxes.end();
	combinedSyntaxes.push_back(preferredTransferSyntax);
	while (s_cur != s_end)
	{
		if (!isaListMember(combinedSyntaxes, *s_cur)) 
			combinedSyntaxes.push_back(*s_cur);
		++s_cur;
	}

	// thin out the sop classes to remove any duplicates.
	OFList<OFString> sops;
	s_cur = sopClasses.begin();
	s_end = sopClasses.end();
	while (s_cur != s_end)
	{
		if (!isaListMember(sops, *s_cur))
		{
			sops.push_back(*s_cur);
		}
		++s_cur;
	}

	// add a presentations context for each sop class / transfer syntax pair
	OFCondition cond = EC_Normal;
	int pid = 1; // presentation context id
	s_cur = sops.begin();
	s_end = sops.end();
	while (s_cur != s_end && cond.good())
	{

		if (pid > 255)
		{
			log.Write("Too many presentation contexts\n");
			return ASC_BADPRESENTATIONCONTEXTID;
		}

		if (opt_combineProposedTransferSyntaxes)
		{
			cond = addPresentationContext(params, pid, *s_cur, combinedSyntaxes);
			pid += 2;   /* only odd presentation context id's */
		}
		else
		{

			// sop class with preferred transfer syntax
			cond = addPresentationContext(params, pid, *s_cur, preferredTransferSyntax);
			pid += 2;   /* only odd presentation context id's */

			if (fallbackSyntaxes.size() > 0)
			{
				if (pid > 255)
				{
					log.Write("Too many presentation contexts\n");
					return ASC_BADPRESENTATIONCONTEXTID;
				}

				// sop class with fallback transfer syntax
				cond = addPresentationContext(params, pid, *s_cur, fallbackSyntaxes);
				pid += 2;       /* only odd presentation context id's */
			}
		}
		++s_cur;
	}

	return cond;
}

bool DICOMSenderImpl::updateStringAttributeValue(DcmItem* dataset, const DcmTagKey& key, OFString value)
{
	DcmStack stack;
	DcmTag tag(key);

	OFCondition cond = EC_Normal;
	cond = dataset->search(key, stack, ESM_fromHere, false);
	if (cond != EC_Normal)
	{
		std::stringstream msg;
		msg << "Error: updateStringAttributeValue: cannot find: " << tag.getTagName() << " " << key << ": " << cond.text() << std::endl;
		log.Write(msg);
		return false;
	}

	DcmElement* elem = (DcmElement*) stack.top();

	DcmVR vr(elem->ident());
	if (elem->getLength() > vr.getMaxValueLength())
	{
		std::stringstream msg;
		msg << "error: updateStringAttributeValue: " << tag.getTagName()
			<< " " << key << ": value too large (max "
			<< vr.getMaxValueLength() << ") for " << vr.getVRName() << " value: " << value << std::endl;
		log.Write(msg);
		return false;
	}

	cond = elem->putOFStringArray(value);
	if (cond != EC_Normal)
	{
		std::stringstream msg;
		msg << "error: updateStringAttributeValue: cannot put string in attribute: " << tag.getTagName()
			<< " " << key << ": " << cond.text() << std::endl;
		log.Write(msg);
		return false;
	}

	return true;
}

void DICOMSenderImpl::replacePatientInfoInformation(DcmDataset* dataset)
{
	std::stringstream msg;		

	if (m_NewPatientID.length() != 0)
	{
		msg << "Changing PatientID from " << m_PatientID << " to " << m_NewPatientID << std::endl;
		log.Write(msg);
		updateStringAttributeValue(dataset, DCM_PatientID, m_NewPatientID.c_str());
	}

	if (m_NewPatientName.length() != 0)
	{
		msg << "Changing PatientName from " << m_PatientName << "to " << m_NewPatientName << std::endl;
		log.Write(msg);
		updateStringAttributeValue(dataset, DCM_PatientName, m_NewPatientName.c_str());
	}

	if (m_NewBirthDay.length() != 0)
	{
		msg << "Changing Birthday from " << m_BirthDay << " to " << m_NewBirthDay << std::endl;
		log.Write(msg);
		updateStringAttributeValue(dataset, DCM_PatientBirthDate, m_NewBirthDay.c_str());
	}
}


void DICOMSenderImpl::progressCallback(void * callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ * req)
{
	DICOMSenderImpl *sender = (DICOMSenderImpl *)callbackData;

	switch (progress->state)
	{
	case DIMSE_StoreBegin:
		sender->log.Write("XMIT:");
		break;
	case DIMSE_StoreEnd:
		sender->log.Write("|\n");
		break;
	default:
		sender->log.Write(".");
		break;
	}
}

OFCondition DICOMSenderImpl::storeSCU(T_ASC_Association * assoc, const boost::filesystem::path &fname)
{
	DIC_US msgId = assoc->nextMsgID++;
	T_DIMSE_C_StoreRQ req;
	T_DIMSE_C_StoreRSP rsp;
	DIC_UI sopClass;
	DIC_UI sopInstance;
	DcmDataset *statusDetail = NULL;	

	std::stringstream msg;
#ifdef _WIN32
	msg << "Sending file: " << fname.string(std::codecvt_utf8<boost::filesystem::path::value_type>()) << "\n";
#else
	msg << "Sending file: " << fname.string() << "\n";
#endif
	log.Write(msg);

	DcmFileFormat dcmff;
	OFCondition cond = dcmff.loadFile(fname.c_str());

	if (cond.bad())
	{		
		log.Write(cond);
		return cond;
	}

	replacePatientInfoInformation(dcmff.getDataset());

	/* figure out which SOP class and SOP instance is encapsulated in the file */
	if (!DU_findSOPClassAndInstanceInDataSet(dcmff.getDataset(), sopClass, sopInstance, false))
	{		
		log.Write("No SOP Class & Instance UIDs\n");
		return DIMSE_BADDATA;
	}

	DcmXfer filexfer(dcmff.getDataset()->getOriginalXfer());

	/* special case: if the file uses an unencapsulated transfer syntax (uncompressed
	* or deflated explicit VR) and we prefer deflated explicit VR, then try
	* to find a presentation context for deflated explicit VR first.
	*/
	if (filexfer.isNotEncapsulated() && opt_networkTransferSyntax == EXS_DeflatedLittleEndianExplicit)
	{
		filexfer = EXS_DeflatedLittleEndianExplicit;
	}

	T_ASC_PresentationContextID presId;
	if (filexfer.getXfer() != EXS_Unknown)
		presId = ASC_findAcceptedPresentationContextID(assoc, sopClass, filexfer.getXferID());
	else 
		presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);

	if (presId == 0)
	{
		const char *modalityName = dcmSOPClassUIDToModality(sopClass);
		if (!modalityName)
			modalityName = dcmFindNameOfUID(sopClass);
		if (!modalityName)
			modalityName = "unknown SOP class";

		std::stringstream msg;
		msg << "No presentation context for: " << sopClass << " = " << modalityName << std::endl;
		log.Write(msg);
		return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
	}

	DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());
	T_ASC_PresentationContext pc;
	ASC_findAcceptedPresentationContext(assoc->params, presId, &pc);
	DcmXfer netTransfer(pc.acceptedTransferSyntax);

	msg << "Transfer: " << dcmFindNameOfUID(fileTransfer.getXferID()) << " -> " << dcmFindNameOfUID(netTransfer.getXferID()) << "\n";
	log.Write(msg);

	if(fileTransfer.getXferID() != netTransfer.getXferID())
	{
		if(dcmff.getDataset()->chooseRepresentation(netTransfer.getXfer(), NULL) != EC_Normal)
		{
			log.Write("Unable to choose Representation\n");
		}
	}

	/* prepare the transmission of data */
	bzero((char*)&req, sizeof(req));
	req.MessageID = msgId;
	strcpy(req.AffectedSOPClassUID, sopClass);
	strcpy(req.AffectedSOPInstanceUID, sopInstance);
	req.DataSetType = DIMSE_DATASET_PRESENT;
	req.Priority = DIMSE_PRIORITY_LOW;	

	// send it!
	cond = DIMSE_storeUser(assoc, presId, &req,
		NULL, dcmff.getDataset(), progressCallback, this,
		DIMSE_NONBLOCKING, 10,
		&rsp, &statusDetail, NULL, boost::filesystem::file_size(fname));	
	
	if (cond.bad())	
	{		
		log.Write("Store Failed\n");
		log.Write(cond);		
	}

	if (statusDetail != NULL)
	{
		msg << "Status Detail:\n";
		statusDetail->print(msg);
		delete statusDetail;
		log.Write(msg);
	}

	// update the sent status
	if (cond.good())
	{
		std::string updatesql = "UPDATE images SET sent = 1 WHERE sopuid = ?";
		sqlite3_stmt *update;
		sqlite3_prepare_v2(db, updatesql.c_str(), updatesql.length(), &update, NULL);
		sqlite3_bind_text(update, 1, sopInstance, strlen(sopInstance), SQLITE_STATIC);
		sqlite3_exec_stmt(update, NULL, NULL, NULL);
		sqlite3_finalize(update);
	}
	return cond;
}

int DICOMSenderImpl::addimage(void *param,int columns,char** values, char**names)
{
	DICOMSenderImpl *sender = (DICOMSenderImpl *) param;

#ifdef _WIN32
	boost::filesystem::path currentFilename(values[0], std::codecvt_utf8<boost::filesystem::path::value_type>());
#else
    boost::filesystem::path currentFilename(values[0]);
#endif

	DcmFileFormat dfile;
	OFCondition cond = dfile.loadFile(currentFilename.c_str());
	if (cond.bad())
	{
		std::stringstream msg;
		msg << "cannot access file, ignoring: " << currentFilename << std::endl;
		sender->log.Write(msg);
		return 0;
	}

	char sopClassUID[128];
	char sopInstanceUID[128];

	if (!DU_findSOPClassAndInstanceInDataSet(dfile.getDataset(), sopClassUID, sopInstanceUID))
	{	
		std::stringstream msg;
		msg << "missing SOP class (or instance) in file, ignoring: " << currentFilename << std::endl;
		sender->log.Write(msg);
		return 0;
	}

	if (!dcmIsaStorageSOPClassUID(sopClassUID))
	{		
		std::stringstream msg;
		msg << "unknown storage sop class in file, ignoring: " << currentFilename << " : " << sopClassUID << std::endl;
		sender->log.Write(msg);
		return 0;
	}

	sender->sopClassUIDList.push_back(sopClassUID);
	sender->fileNameList.push_back(currentFilename);

	return 0;
}

void DICOMSenderImpl::addfiles()
{	
	std::string selectsql = "SELECT filename FROM images WHERE name = ? AND sent = 0";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_text(select, 1, m_PatientName.c_str(), m_PatientName.length(), SQLITE_STATIC);
	sqlite3_exec_stmt(select, &addimage, this, NULL);
	sqlite3_finalize(select);
}

int DICOMSenderImpl::countcallback(void *param,int columns,char** values, char**names)
{
	int *count = (int *) param;

	try { *count = boost::lexical_cast<int>(values[0]); }
	catch(...) { *count = 0; }
	return 0;
}

void DICOMSenderImpl::Cancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = true;
}

bool DICOMSenderImpl::IsDone()
{
	boost::mutex::scoped_lock lk(mutex);
	return doneEvent;
}

bool DICOMSenderImpl::IsCanceled()
{
	boost::mutex::scoped_lock lk(mutex);
	return cancelEvent;
}

void DICOMSenderImpl::SetDone(bool state)
{
	boost::mutex::scoped_lock lk(mutex);
	doneEvent = state;
}

DICOMSender::DICOMSender(void)
{
	impl = new DICOMSenderImpl;
}

DICOMSender::~DICOMSender(void)
{
	delete impl;
}

void DICOMSender::Initialize(sqlite3 *db, const std::string PatientName, std::string PatientID, std::string BirthDay, 
							 std::string NewPatientName, std::string NewPatientID, std::string NewBirthDay,
							 std::string destinationHost, unsigned int destinationPort, std::string destinationAETitle, std::string ourAETitle)
{
	impl->Initialize(db, PatientName, PatientID, BirthDay, NewPatientName, NewPatientID, NewBirthDay,
		destinationHost, destinationPort, destinationAETitle, ourAETitle);
}

void DICOMSender::DoSendThread(void *obj)
{
	DICOMSenderImpl::DoSendThread(((DICOMSender *) obj)->impl);
}

std::string DICOMSender::ReadLog()
{
	return impl->ReadLog();
}

void DICOMSender::Cancel()
{
	impl->Cancel();
}

bool DICOMSender::IsDone()
{
	return impl->IsDone();
}
