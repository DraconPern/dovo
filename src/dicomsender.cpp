
#include "dicomsender.h"
#include "sqlite3_exec_stmt.h"
#include <boost/lexical_cast.hpp>


DICOMSender::DICOMSender(void)
{
	Clear();

	opt_verbose = true;
	opt_showPresentationContexts = false;		
	opt_maxReceivePDULength = ASC_DEFAULTMAXPDU;
	opt_maxSendPDULength = 0;
	opt_networkTransferSyntax = EXS_Unknown;
	// opt_networkTransferSyntax = EXS_JPEG2000LosslessOnly;

	unsuccessfulStoreEncountered = false;
	lastStatusCode = STATUS_Success;

	opt_proposeOnlyRequiredPresentationContexts = false;
	opt_combineProposedTransferSyntaxes = true;

	opt_correctUIDPadding = false;
	patientNamePrefix = "OFFIS^TEST_PN_";   // PatientName is PN (maximum 16 chars)
	patientIDPrefix = "PID_"; // PatientID is LO (maximum 64 chars)
	studyIDPrefix = "SID_";   // StudyID is SH (maximum 16 chars)
	accessionNumberPrefix;  // AccessionNumber is SH (maximum 16 chars)
	opt_secureConnection = false; /* default: no secure connection */    
	opt_blockMode = DIMSE_NONBLOCKING;
	opt_dimse_timeout = 10;
	opt_acse_timeout = 20;
	opt_timeout = 10;

	opt_compressionLevel = 0;
}

DICOMSender::~DICOMSender(void)
{
}

void DICOMSender::Clear(void)
{
	cancelEvent = doneEvent = false;
}

int countcallback(void *param,int columns,char** values, char**names)
{
	int *count = (int *) param;

	*count = boost::lexical_cast<int>(values[0]);
	return 0;
}

void DICOMSender::DoSendThread(void *obj)
{
	DICOMSender *me = (DICOMSender *) obj;
	if (me)
	{
		me->SetDone(false);
		me->DoSend();
		me->SetDone(true);
	}

}

void DICOMSender::WriteLog(const char *msg)
{
	boost::mutex::scoped_lock lk(mycoutmutex);
	mycout << msg;
}

void DICOMSender::WriteLog(const OFCondition &cond)
{
	OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); dumpmsg.append("\n");
	WriteLog(dumpmsg.c_str());
}

void DICOMSender::DoSend()
{

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

		if (countbefore > 0)
		{
			int ret = Send();
		}

		sqlite3_exec_stmt(select, &countcallback, &countafter, NULL);
		sqlite3_finalize(select);

		// only do a sleep if there's more to send, we didn't send anything out, and we still want to retry
		if (countafter > 0 && countbefore == countafter && retry < 10000)
		{
			retry++;
			WriteLog("Waiting 5 mins before retry\n");

			// sleep loop with cancel check, 5 minutes
			int sleeploop = 10 * 5;
			while (sleeploop > 0)
			{
				Sleep(200);
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

int DICOMSender::Send()
{

	addfiles();
	
	OFCondition cond = ASC_initializeNetwork(NET_REQUESTOR, 0, opt_acse_timeout, &net);
	if (cond.bad())
	{		
		WriteLog(cond);
		return 1;
	}

	/* initialize asscociation parameters, i.e. create an instance of T_ASC_Parameters*. */
	cond = ASC_createAssociationParameters(&params, opt_maxReceivePDULength);
	if (cond.bad())
	{		
		WriteLog(cond);
		return 1;
	}
	
	ASC_setAPTitles(params, m_ourAETitle.c_str(), m_destinationAETitle.c_str(), NULL);
	
	cond = ASC_setTransportLayerType(params, false);
	
	/* Figure out the presentation addresses and copy the */
	/* corresponding values into the association parameters.*/
	gethostname(localHost, sizeof(localHost) - 1);
	sprintf(peerHost, "%s:%d", m_destinationHost, m_destinationPort);
	ASC_setPresentationAddresses(params, localHost, peerHost);


	/* Set the presentation contexts which will be negotiated */
	/* when the network connection will be established */
	boost::mutex::scoped_lock lk(mycoutmutex);

	OFListConstIterator(OFString) s_cur = sopClassUIDList.begin();
	OFListConstIterator(OFString) s_end = sopClassUIDList.end();
	while (s_cur != s_end)
	{
		mycout << (*s_cur).c_str();	
		++s_cur;
	}


	cond = addStoragePresentationContexts(params, sopClassUIDList);


	if (cond.bad())
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;
		return 1;
	}

	/* dump presentation contexts if required */
	if (1)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "Request Parameters:\n";
		ASC_dumpParameters(params, mycout);
	}

	/* create association, i.e. try to establish a network connection to another */
	/* DICOM application. This call creates an instance of T_ASC_Association*. */
	if (opt_verbose)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "Requesting Association\n";
	}
	cond = ASC_requestAssociation(net, params, &assoc);
	if (cond.bad())
	{
		if (cond == DUL_ASSOCIATIONREJECTED)
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			T_ASC_RejectParameters rej;

			ASC_getRejectParameters(params, &rej);
			mycout << "Association Rejected:" << std::endl;
			ASC_printRejectParameters(mycout, &rej);
			return 1;
		}
		else
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Association Request Failed:" << std::endl;
			OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;

			return 1;
		}
	}

	/// display the presentation contexts which have been accepted/refused	
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "Association Parameters Negotiated:\n";
		ASC_dumpParameters(params, mycout);
	}

	/* count the presentation contexts which have been accepted by the SCP */
	/* If there are none, finish the execution */
	if (ASC_countAcceptedPresentationContexts(params) == 0)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "No Acceptable Presentation Contexts" << std::endl;
		return 1;
	}

	/* dump general information concerning the establishment of the network connection if required */
	if (opt_verbose)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "Association Accepted (Max Send PDV: "
			<< assoc->sendPDVLength	<< ")\n";
	}

	/* do the real work, i.e. for all files which were specified in the */
	/* command line, transmit the encapsulated DICOM objects to the SCP. */
	cond = EC_Normal;
	OFListIterator(OFString) iter = fileNameList.begin();
	OFListIterator(OFString) enditer = fileNameList.end();

	while ((iter != enditer)/* && (cond == EC_Normal)*/) // compare with EC_Normal since DUL_PEERREQUESTEDRELEASE is also good()
	{
		if (IsCanceled())
			break;
		cond = cstore(assoc, *iter);
		++iter;
	}

	fileNameList.clear();

	/* tear down association, i.e. terminate network connection to SCP */
	if (cond == EC_Normal)
	{
		/* release association */
		if (opt_verbose)
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Releasing Association\n";
		}
		cond = ASC_releaseAssociation(assoc);
		if (cond.bad())
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Association Release Failed:";
			OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;
			return 1;
		}

	}
	else if (cond == DUL_PEERREQUESTEDRELEASE)
	{
		if (1)
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Protocol Error: peer requested release (Aborting)\n";
		}
		if (opt_verbose)
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Aborting Association\n";
		}
		cond = ASC_abortAssociation(assoc);
		if (cond.bad())
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Association Abort Failed:";
			OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;
			return 1;
		}
	}
	else if (cond == DUL_PEERABORTEDASSOCIATION)
	{
		if (opt_verbose)
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Peer Aborted Association\n";
		}
	}
	else
	{
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "SCU Failed:\n";
			OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;
		}
		if (opt_verbose)
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Aborting Association\n";
		}
		cond = ASC_abortAssociation(assoc);
		if (cond.bad())
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Association Abort Failed:";
			OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg ;
			return 1;
		}
	}

	/* destroy the association, i.e. free memory of T_ASC_Association* structure. This */
	/* call is the counterpart of ASC_requestAssociation(...) which was called above. */
	cond = ASC_destroyAssociation(&assoc);
	if (cond.bad())
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;
		return 1;
	}
	/* drop the network, i.e. free memory of T_ASC_Network* structure. This call */
	/* is the counterpart of ASC_initializeNetwork(...) which was called above. */
	cond = ASC_dropNetwork(&net);
	if (cond.bad())
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;
		return 1;
	}

	return 0;
}


static bool
	isaListMember(OFList<OFString>& lst, OFString& s)
{
	OFListIterator(OFString) cur = lst.begin();
	OFListIterator(OFString) end = lst.end();

	bool found = false;

	while (cur != end && !found)
	{

		found = (s == *cur);

		++cur;
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

OFCondition DICOMSender::addStoragePresentationContexts(T_ASC_Parameters *params, OFList<OFString>& sopClasses)
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
		if (!isaListMember(combinedSyntaxes, *s_cur)) combinedSyntaxes.push_back(*s_cur);
		++s_cur;
	}

	if (!opt_proposeOnlyRequiredPresentationContexts)
	{
		// add the (short list of) known storage sop classes to the list
		// the array of Storage SOP Class UIDs comes from dcuid.h
		for (int i=0; i<numberOfDcmShortSCUStorageSOPClassUIDs; i++)
		{
			sopClasses.push_back(dcmShortSCUStorageSOPClassUIDs[i]);
		}
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
			boost::mutex::scoped_lock lk(mycoutmutex);
			mycout << "Too many presentation contexts\n";
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
					boost::mutex::scoped_lock lk(mycoutmutex);
					mycout << "Too many presentation contexts";
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

bool DICOMSender::updateStringAttributeValue(DcmItem* dataset, const DcmTagKey& key, OFString value)
{
	DcmStack stack;
	DcmTag tag(key);

	OFCondition cond = EC_Normal;
	cond = dataset->search(key, stack, ESM_fromHere, false);
	if (cond != EC_Normal)
	{
		mycout << "error: updateStringAttributeValue: cannot find: " << tag.getTagName() << " " << key << ": " << cond.text() << std::endl;
		return false;
	}

	DcmElement* elem = (DcmElement*) stack.top();

	DcmVR vr(elem->ident());
	if (elem->getLength() > vr.getMaxValueLength())
	{
		mycout << "error: updateStringAttributeValue: " << tag.getTagName()
			<< " " << key << ": value too large (max "
			<< vr.getMaxValueLength() << ") for " << vr.getVRName() << " value: " << value << std::endl;
		return false;
	}

	cond = elem->putOFStringArray(value);
	if (cond != EC_Normal)
	{
		mycout << "error: updateStringAttributeValue: cannot put string in attribute: " << tag.getTagName()
			<< " " << key << ": "
			<< cond.text() << std::endl;
		return false;
	}

	return true;
}

void DICOMSender::replacePatientInfoInformation(DcmDataset* dataset)
{
	if (opt_verbose)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		//mycout << "  PatientName=" << m_PatientName << std::endl;
		mycout << "Changing PatientID=" << m_NewPatientID << std::endl;
		mycout << "Changing PatientName=" << m_NewPatientName << std::endl;
		mycout << "Changing Birthday=" << m_NewBirthDay << std::endl;
	}

	//
	if (m_NewPatientID.length() != 0)
		updateStringAttributeValue(dataset, DCM_PatientID, m_NewPatientID.c_str());

	if (m_NewPatientName.length() != 0)
		updateStringAttributeValue(dataset, DCM_PatientName, m_NewPatientName.c_str());

	if (m_NewBirthDay.length() != 0)
		updateStringAttributeValue(dataset, DCM_PatientBirthDate, m_NewBirthDay.c_str());
}


void DICOMSender::progressCallback(void * callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ * req)
{
	DICOMSender *sender = (DICOMSender *)callbackData;
	if (sender->opt_verbose)
	{
		boost::mutex::scoped_lock lk(sender->mycoutmutex);
		switch (progress->state)
		{
		case DIMSE_StoreBegin:
			sender->mycout << "XMIT:";
			break;
		case DIMSE_StoreEnd:
			sender->mycout << "|\n";
			break;
		default:
			sender->mycout << '.';
			break;
		}
	}

	if (sender->IsCanceled())
	{
		DIMSE_sendCancelRequest(sender->assoc, sender->presId, req->MessageID);
	}
}

OFCondition DICOMSender::storeSCU(T_ASC_Association * assoc, const char *fname)
	/*
	* This function will read all the information from the given file,
	* figure out a corresponding presentation context which will be used
	* to transmit the information over the network to the SCP, and it
	* will finally initiate the transmission of all data to the SCP.
	*
	* Parameters:
	*   assoc - [in] The association (network connection to another DICOM application).
	*   fname - [in] Name of the file which shall be processed.
	*/
{
	DIC_US msgId = assoc->nextMsgID++;
	T_DIMSE_C_StoreRQ req;
	T_DIMSE_C_StoreRSP rsp;
	DIC_UI sopClass;
	DIC_UI sopInstance;
	DcmDataset *statusDetail = NULL;

	unsuccessfulStoreEncountered = true; // assumption

	if (opt_verbose)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "--------------------------\n";
		mycout << "Sending file: " << fname << "\n";
	}

	/* read information from file. After the call to DcmFileFormat::loadFile(...) the information */
	/* which is encapsulated in the file will be available through the DcmFileFormat object. */
	/* In detail, it will be available through calls to DcmFileFormat::getMetaInfo() (for */
	/* meta header information) and DcmFileFormat::getDataset() (for data set information). */
	DcmFileFormat dcmff;
	OFCondition cond = dcmff.loadFile(fname);

	/* figure out if an error occured while the file was read*/
	if (cond.bad())
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "Bad DICOM file: " << fname << ":" << cond.text() << std::endl;
		return cond;
	}

	replacePatientInfoInformation(dcmff.getDataset());

	/* figure out which SOP class and SOP instance is encapsulated in the file */
	if (!DU_findSOPClassAndInstanceInDataSet(dcmff.getDataset(),
		sopClass, sopInstance, opt_correctUIDPadding))
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "No SOP Class & Instance UIDs in file: " << fname << std::endl;
		return DIMSE_BADDATA;
	}

	/* figure out which of the accepted presentation contexts should be used */
	DcmXfer filexfer(dcmff.getDataset()->getOriginalXfer());

	/* special case: if the file uses an unencapsulated transfer syntax (uncompressed
	* or deflated explicit VR) and we prefer deflated explicit VR, then try
	* to find a presentation context for deflated explicit VR first.
	*/
	if (filexfer.isNotEncapsulated() &&
		opt_networkTransferSyntax == EXS_DeflatedLittleEndianExplicit)
	{
		filexfer = EXS_DeflatedLittleEndianExplicit;
	}

	if (filexfer.getXfer() != EXS_Unknown) presId = ASC_findAcceptedPresentationContextID(assoc, sopClass, filexfer.getXferID());
	else presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);
	if (presId == 0)
	{
		const char *modalityName = dcmSOPClassUIDToModality(sopClass);
		if (!modalityName) modalityName = dcmFindNameOfUID(sopClass);
		if (!modalityName) modalityName = "unknown SOP class";
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "No presentation context for: " << modalityName << ", " << sopClass << std::endl;
		return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
	}


	DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());
	T_ASC_PresentationContext pc;
	ASC_findAcceptedPresentationContext(assoc->params, presId, &pc);
	DcmXfer netTransfer(pc.acceptedTransferSyntax);

	/* if required, dump general information concerning transfer syntaxes */
	if (opt_verbose)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "Transfer: "
			<< dcmFindNameOfUID(fileTransfer.getXferID()) << " -> "
			<< dcmFindNameOfUID(netTransfer.getXferID()) << "\n";
	}

	if(fileTransfer.getXferID() != netTransfer.getXferID())
	{
		if(dcmff.getDataset()->chooseRepresentation(netTransfer.getXfer(), NULL) != EC_Normal)
		{

		}
	}

	/* prepare the transmission of data */
	bzero((char*)&req, sizeof(req));
	req.MessageID = msgId;
	strcpy(req.AffectedSOPClassUID, sopClass);
	strcpy(req.AffectedSOPInstanceUID, sopInstance);
	req.DataSetType = DIMSE_DATASET_PRESENT;
	req.Priority = DIMSE_PRIORITY_LOW;

	/* if required, dump some more general information */
	if (opt_verbose)
	{
		mycout << "Store SCU RQ: MsgID " <<  msgId << ", " << dcmSOPClassUIDToModality(sopClass) << std::endl;
	}

	/* finally conduct transmission of data */
	cond = DIMSE_storeUser(assoc, presId, &req,
		NULL, dcmff.getDataset(), progressCallback, this,
		opt_blockMode, opt_dimse_timeout,
		&rsp, &statusDetail, NULL, OFStandard::getFileSize(fname));

	/*
	* If store command completed normally, with a status
	* of success or some warning then the image was accepted.
	*/
	if (cond == EC_Normal && (rsp.DimseStatus == STATUS_Success || DICOM_WARNING_STATUS(rsp.DimseStatus)))
	{
		unsuccessfulStoreEncountered = false;
	}

	/* remember the response's status for later transmissions of data */
	lastStatusCode = rsp.DimseStatus;

	/* dump some more general information */
	if (cond == EC_Normal)
	{
		if (opt_verbose)
		{
			boost::mutex::scoped_lock lk(mycoutmutex);
			OFString dumpmsg;
			DIMSE_dumpMessage(dumpmsg, rsp, DIMSE_INCOMING);
			mycout << dumpmsg << std::endl;
		}
	}
	else
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "Store Failed, file: " << fname << std::endl;
		OFString dumpmsg; DimseCondition::dump(dumpmsg, cond); mycout << dumpmsg << std::endl;
	}

	/* dump status detail information if there is some */
	if (statusDetail != NULL)
	{
		boost::mutex::scoped_lock lk(mycoutmutex);
		mycout << "  Status Detail:\n";
		statusDetail->print(mycout);
		delete statusDetail;
	}
	/* return */
	return cond;
}


OFCondition DICOMSender::cstore(T_ASC_Association * assoc, const OFString& fname)
	/*
	* This function will process the given file as often as is specified by opt_repeatCount.
	* "Process" in this case means "read file, send C-STORE-RQ, receive C-STORE-RSP".
	*
	* Parameters:
	*   assoc - [in] The association (network connection to another DICOM application).
	*   fname - [in] Name of the file which shall be processed.
	*/
{
	OFCondition cond = EC_Normal;

	/* process file (read file, send C-STORE-RQ, receive C-STORE-RSP) */
	cond = storeSCU(assoc, fname.c_str());    

	// update the sent status
	if (cond.good())
	{
		std::string updatesql = "UPDATE images SET sent = 1 WHERE filename = ?";
		sqlite3_stmt *update;
		sqlite3_prepare_v2(db, updatesql.c_str(), updatesql.length(), &update, NULL);
		sqlite3_bind_text(update, 1, fname.c_str(), fname.length(), SQLITE_STATIC);
		sqlite3_exec_stmt(update, NULL, NULL, NULL);
		sqlite3_finalize(update);
	}

	return cond;
}

int DICOMSender::addimage(void *param,int columns,char** values, char**names)
{
	DICOMSender *sender = (DICOMSender *) param;

	char *currentFilename = values[0];

	bool ignoreName = false;
	OFString errormsg;
	char sopClassUID[128];
	char sopInstanceUID[128];

	if (access(currentFilename, R_OK) < 0)
	{
		errormsg = "cannot access file: ";
		errormsg += currentFilename;
		boost::mutex::scoped_lock lk(sender->mycoutmutex);
		sender->mycout << "warning: " << errormsg << ", ignoring file" << std::endl;
	}
	else
	{
		if (sender->opt_proposeOnlyRequiredPresentationContexts)
		{
			if (!DU_findSOPClassAndInstanceInFile(currentFilename, sopClassUID, sopInstanceUID))
			{
				ignoreName = true;
				errormsg = "missing SOP class (or instance) in file: ";
				errormsg += currentFilename;
				boost::mutex::scoped_lock lk(sender->mycoutmutex);
				sender->mycout << "warning: " << errormsg << ", ignoring file" << std::endl;
			}
			else if (!dcmIsaStorageSOPClassUID(sopClassUID))
			{
				ignoreName = true;
				errormsg = "unknown storage sop class in file: ";
				errormsg += currentFilename;
				errormsg += ": ";
				errormsg += sopClassUID;
				boost::mutex::scoped_lock lk(sender->mycoutmutex);
				sender->mycout << "warning: " << errormsg << ", ignoring file" << std::endl;
			}
			else
			{
				sender->sopClassUIDList.push_back(sopClassUID);
			}
		}

		if (!ignoreName)
			sender->fileNameList.push_back(currentFilename);
	}
	return 0;
}

void DICOMSender::addfiles()
{	
	std::string selectsql = "SELECT filename FROM images WHERE name = ? AND sent = 0";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_text(select, 1, m_PatientName.c_str(), m_PatientName.length(), SQLITE_STATIC);
	sqlite3_exec_stmt(select, &addimage, this, NULL);
	sqlite3_finalize(select);
}

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