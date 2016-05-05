
#include "dicomsender.h"
#include <boost/lexical_cast.hpp>
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

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif


class DICOMSenderImpl
{	

public:
	DICOMSenderImpl(PatientData &patientdata);
	~DICOMSenderImpl(void);

	void DoSendAsync(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);	
	void DoSend(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);	

	static bool Echo(DestinationEntry destination);

	std::string ReadLog();
	void WriteLog(const char *msg);
	void WriteLog(std::string &msg);

	void Cancel();
	bool IsDone();				
protected:
	static void DoSendThread(void *obj);
	PatientData &patientdata;
	
	int SendABatch();

	bool IsCanceled();
	void SetDone(bool state);

	// threading data
	boost::mutex mutex;
	bool cancelEvent, doneEvent;
	std::string PatientID;
	std::string PatientName;
	bool changeinfo;	
	std::string NewPatientID;
	std::string NewPatientName;
	std::string NewBirthDay;
	DestinationEntry m_destination;	
	
	typedef std::map<std::string, std::set<std::string> > mapset;
	mapset sopclassuidtransfersyntax;

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

	typedef std::map<std::string, boost::filesystem::path, doj::alphanum_less<std::string> > naturalpathmap;	
	int fillstudies(Study &study);	
	int fillseries(Series &series);
	int fillinstances(Instance &instance, naturalpathmap *entries);
	std::vector<std::string> studies, series;
	naturalpathmap instances;	// sopid, filename, this ensures we send out instances in sopid order	
};


DICOMSenderImpl::DICOMSenderImpl(PatientData &patientdata) 
	: patientdata(patientdata)
{			
	cancelEvent = doneEvent = false;
}

DICOMSenderImpl::~DICOMSenderImpl()
{
}

void DICOMSenderImpl::DoSendAsync(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination)
{
	cancelEvent = doneEvent = false;
	
	this->PatientID = PatientID;
	this->PatientName = PatientName;
	this->changeinfo = changeinfo;
	this->NewPatientID = NewPatientID;
	this->NewPatientName = NewPatientName;	
	this->NewBirthDay = NewBirthDay;
	this->m_destination = destination;	

	// start the thread, let the sender manage (e.g. cancel), so we don't need to track anymore
	boost::thread t(DICOMSenderImpl::DoSendThread, this);
	t.detach();
}

class MyAppender : public dcmtk::log4cplus::Appender 
{
public:      
    MyAppender(DICOMSenderImpl &sender) : sender(sender) {}              
	virtual void close() { closed = true; }
protected:
	virtual void append(const dcmtk::log4cplus::spi::InternalLoggingEvent& event)
	{		
		sender.WriteLog(formatEvent(event).c_str());
	}

    DICOMSenderImpl &sender;
};

void DICOMSenderImpl::DoSendThread(void *obj)
{
	DICOMSenderImpl *me = (DICOMSenderImpl *) obj;
	dcmtk::log4cplus::SharedAppenderPtr stringlogger(new MyAppender(*me));
	dcmtk::log4cplus::Logger rootLogger = dcmtk::log4cplus::Logger::getRoot();
	rootLogger.removeAllAppenders();
	rootLogger.addAppender(stringlogger);

	OFLog::configure(OFLogger::INFO_LOG_LEVEL);

	if (me)
	{
		me->SetDone(false);
		me->DoSend(me->PatientID, me->PatientName, me->changeinfo, me->NewPatientID, me->NewPatientName, me->NewBirthDay, me->m_destination);
		me->SetDone(true);
	}

	rootLogger.removeAllAppenders();	
	dcmtk::log4cplus::threadCleanup();
}

std::string DICOMSenderImpl::ReadLog()
{
	return log.Read();
}

void DICOMSenderImpl::WriteLog(const char *msg)
{
	log.Write(msg);
}

void DICOMSenderImpl::WriteLog(std::string &msg)
{
	log.Write(msg.c_str());
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

class DcmDatasetEditor
{
public:
	DcmDatasetEditor(DcmDataset* dataset) : dataset(dataset) {};
	bool updateStringAttributeValue(DcmItem* dataset, const DcmTagKey& key, std::string value);
	void replacePatientInfoInformation(std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay);
protected:
	DcmDataset* dataset;
};

void DICOMSenderImpl::DoSend(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination)
{	
	// get a list of files
	patientdata.GetStudies(PatientID, PatientName, boost::bind(&DICOMSenderImpl::fillstudies, this, _1));
	for (std::vector<std::string>::iterator it = studies.begin() ; it != studies.end(); ++it)
	{
		patientdata.GetSeries(*it, boost::bind(&DICOMSenderImpl::fillseries, this, _1));
		for (std::vector<std::string>::iterator it2 = series.begin() ; it2 != series.end(); ++it2)
		{
			patientdata.GetInstances(*it2, boost::bind(&DICOMSenderImpl::fillinstances, this, _1, &instances));
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

int DICOMSenderImpl::fillstudies(Study &study)
{
	studies.push_back(study.studyuid);
	return 0;
}

int DICOMSenderImpl::fillseries(Series &series)
{
	this->series.push_back(series.seriesuid);
	return 0;
}

int DICOMSenderImpl::fillinstances(Instance &instance, naturalpathmap *entries)
{
	// add file to send
	entries->insert(std::pair<std::string, boost::filesystem::path>(instance.sopuid, instance.filename));

	// remember the class and transfersyntax
	sopclassuidtransfersyntax[instance.sopclassuid].insert(instance.transfersyntax);
	return 0;
}

class MyDcmSCU: public DcmSCU
{
public:
	MyDcmSCU(DICOMSenderImpl &sender) : sender(sender) {}
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

	DICOMSenderImpl &sender;	
};

int DICOMSenderImpl::SendABatch()
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

	naturalpathmap::iterator itr = instances.begin();
	while(itr != instances.end())
	{
		if(IsCanceled())
		{
			log.Write("Send canceled\n");
			break;
		}

		Uint16 status;

		std::stringstream msg;
		msg << "Sending file: " << itr->second << "\n";
		log.Write(msg);
		scu.newtransfer = true;

		// load file
		DcmFileFormat dcmff;
		dcmff.loadFile(itr->second.c_str());

		if(changeinfo)
		{
			dcmff.getDataset()->putAndInsertString(DCM_PatientID, NewPatientID.c_str());
			dcmff.getDataset()->putAndInsertString(DCM_PatientName, NewPatientName.c_str());
			dcmff.getDataset()->putAndInsertString(DCM_PatientBirthDate, NewBirthDay.c_str());
		}

		// do some precheck of the transfer syntax
		DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());
		OFString sopclassuid;
		dcmff.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopclassuid);

		msg << "File encoding: " << fileTransfer.getXferName() << "\n";
		log.Write(msg);

		// out found.. change to 
		T_ASC_PresentationContextID pid = scu.findAnyPresentationContextID(sopclassuid, fileTransfer.getXferID());
		
		cond = scu.sendSTORERequest(pid, "", dcmff.getDataset(), status);
		if(cond.good())
			instances.erase(itr++);
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

bool DICOMSenderImpl::Echo(DestinationEntry destination)
{
	DcmSCU scu;

	scu.setVerbosePCMode(true);
	scu.setAETitle(destination.ourAETitle.c_str());
	scu.setPeerHostName(destination.destinationHost.c_str());
	scu.setPeerPort(destination.destinationPort);
	scu.setPeerAETitle(destination.destinationAETitle.c_str());
	scu.setACSETimeout(30);
	scu.setDIMSETimeout(60);
	scu.setDatasetConversionMode(true);
	
	OFList<OFString> transfersyntax;
	transfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);
	transfersyntax.push_back(UID_LittleEndianImplicitTransferSyntax);
	scu.addPresentationContext(UID_VerificationSOPClass, transfersyntax);

	OFCondition cond;
	cond = scu.initNetwork();	
	if(cond.bad())	
		return false;
	
	cond = scu.negotiateAssociation();
	if(cond.bad())
		return false;

	cond = scu.sendECHORequest(0);

	scu.releaseAssociation();

	if(cond == EC_Normal)
	{
		return true;
	}

	return false;
}

bool DcmDatasetEditor::updateStringAttributeValue(DcmItem* dataset, const DcmTagKey& key, std::string value)
{
	DcmStack stack;
	DcmTag tag(key);

	OFCondition cond = EC_Normal;
	cond = dataset->search(key, stack, ESM_fromHere, false);
	if (cond != EC_Normal)
	{		
		return false;
	}

	DcmElement* elem = (DcmElement*) stack.top();

	DcmVR vr(elem->ident());
	if (elem->getLength() > vr.getMaxValueLength())
	{	
		return false;
	}

	cond = elem->putOFStringArray(value.c_str());
	if (cond != EC_Normal)
	{	
		return false;
	}

	return true;
}

void DcmDatasetEditor::replacePatientInfoInformation(std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay)
{
	if (NewPatientID.length() != 0)
		updateStringAttributeValue(dataset, DCM_PatientID, NewPatientID);

	if (NewPatientName.length() != 0)
		updateStringAttributeValue(dataset, DCM_PatientName, NewPatientName);

	if (NewBirthDay.length() != 0)
		updateStringAttributeValue(dataset, DCM_PatientBirthDate, NewBirthDay);
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


DICOMSender::DICOMSender(PatientData &patientdata)
{
	impl = new DICOMSenderImpl(patientdata);
}

DICOMSender::~DICOMSender(void)
{
	delete impl;
}

void DICOMSender::DoSendAsync(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination)
{
	impl->DoSendAsync(PatientID, PatientName, changeinfo, NewPatientID, NewPatientName, NewBirthDay, destination);
}

void DICOMSender::DoSend(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination)
{
	impl->DoSend(PatientID, PatientName, changeinfo, NewPatientID, NewPatientName, NewBirthDay, destination);
}

bool DICOMSender::Echo(DestinationEntry destination)
{
	return DICOMSenderImpl::Echo(destination);
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