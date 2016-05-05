#ifndef _DICOMSENDER_
#define _DICOMSENDER_

#include "alphanum.hpp"
#include <set>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include "patientdata.h"
#include "destinationentry.h"

class DICOMSenderImpl;
class DICOMSender
{	

public:
	DICOMSender(PatientData &patientdata);
	~DICOMSender(void);

	void DoSendAsync(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);	
	void DoSend(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);	
	static bool Echo(DestinationEntry destination);
	std::string ReadLog();
	void WriteLog(const char *msg);
	void WriteLog(std::string &msg);

	void Cancel();
	bool IsDone();				
protected:
	DICOMSenderImpl *impl;
};

#endif