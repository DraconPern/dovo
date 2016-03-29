#ifndef _DICOMSENDER_
#define _DICOMSENDER_

#include "alphanum.hpp"
#include <set>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include "patientdata.h"
#include "destinationentry.h"

// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif

#include "dcmtk/ofstd/ofcond.h"

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif


class DICOMSender
{	

public:
	DICOMSender(PatientData &patientdata);
	~DICOMSender(void);

	// typedef std::set<boost::filesystem::path, doj::alphanum_less<boost::filesystem::path> > naturalset;	

	// void SetFileList(const naturalset &files);

	void DoSendAsync(std::string PatientID, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);	
	void DoSend(std::string PatientID, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);	

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

	typedef std::map<std::string, std::string, doj::alphanum_less<std::string> > naturalmap;	
	int fillstudies(Study &study);	
	int fillseries(Series &series);
	int fillinstances(Instance &instance, naturalmap *entries);
	std::vector<std::string> studies, series;
	naturalmap instances;	// sopid, filename, this ensures we send out instances in sopid order
};

#endif