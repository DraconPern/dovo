
#include "dicomsender.h"
#include <boost/lexical_cast.hpp>
#include <codecvt>
#include <boost/thread.hpp>
#include "destinationentry.h"

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
#include "dcmtk/dcmnet/scu.h"

// check DCMTK functionality
#if !defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
#error "DCMTK and this program must be compiled with DCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS"
#endif

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif



DICOMSender::DICOMSender(PatientData &patientdata) 
	: patientdata(patientdata)
{			
	cancelEvent = doneEvent = false;
}

DICOMSender::~DICOMSender()
{
}

void DICOMSender::DoSendAsync(std::string PatientID, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination)
{
	cancelEvent = doneEvent = false;
	
	this->PatientID = PatientID;
	this->changeinfo = changeinfo;
	this->NewPatientID = NewPatientID;
	this->NewPatientName = NewPatientName;	
	this->NewBirthDay = NewBirthDay;
	this->m_destination = destination;	

	// start the thread, let the sender manage (e.g. cancel), so we don't need to track anymore
	boost::thread t(DICOMSender::DoSendThread, this);
	t.detach();
}

class MyAppender : public dcmtk::log4cplus::Appender 
{
public:      
    MyAppender(DICOMSender &sender) : sender(sender) {}              
	virtual void close() { closed = true; }
protected:
	virtual void append(const dcmtk::log4cplus::spi::InternalLoggingEvent& event)
	{		
		sender.WriteLog(formatEvent(event).c_str());
	}

    DICOMSender &sender;
};

void DICOMSender::DoSendThread(void *obj)
{
	DICOMSender *me = (DICOMSender *) obj;
	dcmtk::log4cplus::SharedAppenderPtr stringlogger(new MyAppender(*me));
	dcmtk::log4cplus::Logger rootLogger = dcmtk::log4cplus::Logger::getRoot();
	rootLogger.removeAllAppenders();
	rootLogger.addAppender(stringlogger);

	OFLog::configure(OFLogger::INFO_LOG_LEVEL);

	if (me)
	{
		me->SetDone(false);
		me->DoSend(me->PatientID, me->changeinfo, me->NewPatientID, me->NewPatientName, me->NewBirthDay, me->m_destination);
		me->SetDone(true);
	}

	rootLogger.removeAllAppenders();	
	dcmtk::log4cplus::threadCleanup();
}

std::string DICOMSender::ReadLog()
{
	return log.Read();
}

void DICOMSender::WriteLog(const char *msg)
{
	log.Write(msg);
}

void DICOMSender::WriteLog(std::string &msg)
{
	log.Write(msg.c_str());
}

void DICOMSender::GUILog::Write(const char *msg)
{
	boost::mutex::scoped_lock lk(mycoutmutex);
	mycout << msg;
}

void DICOMSender::GUILog::Write(const OFCondition &cond)
{
	OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); dumpmsg.append("\n");
	Write(dumpmsg.c_str());
}

void DICOMSender::GUILog::Write(std::stringstream &stream)
{	
	Write(stream.str().c_str());
	stream.str(std::string());
}

std::string DICOMSender::GUILog::Read()
{
	boost::mutex::scoped_lock lk(mycoutmutex);	
	std::string str = mycout.str();
	mycout.str("");
	return str;
}

void DICOMSender::DoSend(std::string PatientID, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination)
{	
	// get a list of files
	patientdata.GetStudies(PatientID, boost::bind(&DICOMSender::fillstudies, this, _1));
	for (std::vector<std::string>::iterator it = studies.begin() ; it != studies.end(); ++it)
	{
		patientdata.GetSeries(*it, boost::bind(&DICOMSender::fillseries, this, _1));
		for (std::vector<std::string>::iterator it2 = series.begin() ; it2 != series.end(); ++it2)
		{
			patientdata.GetInstances(*it2, boost::bind(&DICOMSender::fillinstances, this, _1, &instances));
		}
	}
		
	std::stringstream msg;
	msg << "Sending " << instances.size() << " images\n";
	log.Write(msg);

	int retry = 0;
	int unsentcountbefore = 0;
	int unsentcountafter = 0;
	do
	{
		// get number of unsent images
		unsentcountbefore = instances.size();

		// batch send
		if (unsentcountbefore > 0)
			SendABatch();		

		unsentcountafter = instances.size();
		
		// only do a sleep if there's more to send, we didn't send anything out, and we still want to retry
		if (unsentcountafter > 0 && unsentcountbefore == unsentcountafter && retry < 10000)
		{
			retry++;			
			msg << unsentcountafter << " images left to send\n";
			log.Write(msg);
			log.Write("Waiting 1 mins before retry\n");

			// sleep loop with cancel check, 1 minutes
			int sleeploop = 5 * 60 * 1;
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
	while (!IsCanceled() && unsentcountafter > 0 && retry < 10000);	 
}

int DICOMSender::fillstudies(Study &study)
{
	studies.push_back(study.studyuid);
	return 0;
}

int DICOMSender::fillseries(Series &series)
{
	this->series.push_back(series.seriesuid);
	return 0;
}

int DICOMSender::fillinstances(Instance &instance, naturalmap *entries)
{
	// add file to send
	entries->insert(std::pair<std::string, std::string>(instance.sopuid, instance.filename));

	// remember the class and transfersyntax
	sopclassuidtransfersyntax[instance.sopclassuid].insert(instance.transfersyntax);
	return 0;
}

class MyDcmSCU: public DcmSCU
{
public:
	MyDcmSCU(DICOMSender &sender) : sender(sender) {}
	bool newtransfer;
protected:
	virtual void notifySENDProgress(const unsigned long byteCount)
	{
		/*if(newtransfer)
		{
			sender.WriteLog("|");
			newtransfer = false;
		}
		else
			sender.WriteLog(".");*/
	}

	DICOMSender &sender;	
};

int DICOMSender::SendABatch()
{		
	MyDcmSCU scu(*this);

	scu.setVerbosePCMode(true);
	scu.setAETitle(m_destination.ourAETitle.c_str());
	scu.setPeerHostName(m_destination.destinationHost.c_str());
	scu.setPeerPort(m_destination.destinationPort);
	scu.setPeerAETitle(m_destination.destinationAETitle.c_str());
	scu.setACSETimeout(30);
	scu.setDIMSETimeout(60);
	scu.setDatasetConversionMode(true);

	OFList<OFString> defaulttransfersyntax;
	defaulttransfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);

	// for every class..
	for(mapset::iterator it = sopclassuidtransfersyntax.begin(); it != sopclassuidtransfersyntax.end(); it++)
	{
		// make list of what's in the file, and propose it first.  default proposed as a seperate context
		OFList<OFString> transfersyntax;
		for(std::set<std::string>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			if(*it2 != UID_LittleEndianExplicitTransferSyntax)
				transfersyntax.push_back(it2->c_str());
		}

		if(transfersyntax.size() > 0)
			scu.addPresentationContext(it->first.c_str(), transfersyntax);

		// propose the default UID_LittleEndianExplicitTransferSyntax
		scu.addPresentationContext(it->first.c_str(), defaulttransfersyntax);
	}
	
	OFCondition cond;
	
	if(scu.initNetwork().bad())
		return 1;

	if(scu.negotiateAssociation().bad())
		return 1;

	naturalmap::iterator itr = instances.begin();
	while(itr != instances.end())
	{
		Uint16 status;

		std::stringstream msg;
		msg << "Sending file: " << itr->second << "\n";
		log.Write(msg);
		scu.newtransfer = true;
		cond = scu.sendSTORERequest(0, itr->second.c_str(), NULL, status);
		if(cond.good())
			itr = instances.erase(itr);
		else if(cond == DUL_PEERABORTEDASSOCIATION)
			return 1;
		else			// some error? keep going
		{		
			itr++;
		}
		/*if(!scu.newtransfer)
			log.Write("|");*/
		
		log.Write("\n");
	}

	scu.releaseAssociation();	
	return 0;
}
/*

bool DICOMSender::updateStringAttributeValue(DcmItem* dataset, const DcmTagKey& key, std::string value)
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

	cond = elem->putOFStringArray(value.c_str());
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

void DICOMSender::replacePatientInfoInformation(DcmDataset* dataset)
{
	std::stringstream msg;

	if(changeinfo)
	{
		if (NewPatientID.length() != 0)
		{
			msg << "Changing PatientID to " << NewPatientID << std::endl;
			log.Write(msg);
			updateStringAttributeValue(dataset, DCM_PatientID, NewPatientID);
		}

		if (NewPatientName.length() != 0)
		{
			msg << "Changing PatientName to " << NewPatientName << std::endl;
			log.Write(msg);
			updateStringAttributeValue(dataset, DCM_PatientName, NewPatientName);
		}

		if (NewBirthDay.length() != 0)
		{
			msg << "Changing Birthday to " << NewBirthDay << std::endl;
			log.Write(msg);
			updateStringAttributeValue(dataset, DCM_PatientBirthDate, NewBirthDay);
		}
	}
}

*/
/*

void DICOMSender::progressCallback(void * callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ * req)
{
	DICOMSender *sender = (DICOMSender *)callbackData;

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

OFCondition DICOMSender::storeSCU(T_ASC_Association * assoc, const boost::filesystem::path &fname)
{
	DIC_US msgId = assoc->nextMsgID++;
	T_DIMSE_C_StoreRQ req;
	T_DIMSE_C_StoreRSP rsp;
	DIC_UI sopClass;
	DIC_UI sopInstance;
	DcmDataset *statusDetail = NULL;	

	std::stringstream msg;
#ifdef _WIN32
	// on Windows, boost::filesystem::path is a wstring, so we need to convert to utf8
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

	if (!DU_findSOPClassAndInstanceInDataSet(dcmff.getDataset(), sopClass, sopInstance, false))
	{		
		log.Write("No SOP Class & Instance UIDs\n");
		return DIMSE_BADDATA;
	}

	DcmXfer filexfer(dcmff.getDataset()->getOriginalXfer());

	// special case: if the file uses an unencapsulated transfer syntax (uncompressed
    // or deflated explicit VR) and we prefer deflated explicit VR, then try
	 // to find a presentation context for deflated explicit VR first.
	
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

	
	bzero((char*)&req, sizeof(req));
	req.MessageID = msgId;
	strcpy(req.AffectedSOPClassUID, sopClass);
	strcpy(req.AffectedSOPInstanceUID, sopInstance);
	req.DataSetType = DIMSE_DATASET_PRESENT;
	req.Priority = DIMSE_PRIORITY_LOW;	

	// send it!
	cond = DIMSE_storeUser(assoc, presId, &req,
		NULL, dcmff.getDataset(), progressCallback, this,
		DIMSE_NONBLOCKING, 60,
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

	
	return cond;
}

bool DICOMSender::scanFile(boost::filesystem::path currentFilename)
{	

	DcmFileFormat dfile;
	OFCondition cond = dfile.loadFile(currentFilename.c_str());
	if (cond.bad())
	{
		std::stringstream msg;
		msg << "cannot access file, ignoring: " << currentFilename << std::endl;
		log.Write(msg);
		return true;
	}

	char sopClassUID[128];
	char sopInstanceUID[128];

	if (!DU_findSOPClassAndInstanceInDataSet(dfile.getDataset(), sopClassUID, sopInstanceUID))
	{	
		std::stringstream msg;
		msg << "missing SOP class (or instance) in file, ignoring: " << currentFilename << std::endl;
		log.Write(msg);
		return false;
	}

	if (!dcmIsaStorageSOPClassUID(sopClassUID))
	{		
		std::stringstream msg;
		msg << "unknown storage sop class in file, ignoring: " << currentFilename << " : " << sopClassUID << std::endl;
		log.Write(msg);
		return false;
	}

	sopClassUIDList.push_back(sopClassUID);	

	return true;
}
*/
void DICOMSender::Cancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = true;
}

bool DICOMSender::IsDone()
{
	boost::mutex::scoped_lock lk(mutex);
	return doneEvent;
}

bool DICOMSender::IsCanceled()
{
	boost::mutex::scoped_lock lk(mutex);
	return cancelEvent;
}

void DICOMSender::SetDone(bool state)
{
	boost::mutex::scoped_lock lk(mutex);
	doneEvent = state;
}
