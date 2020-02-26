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

	void DoSendPatientAsync(std::string PatientID, std::string PatientName, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);		
	void DoSendStudyAsync(std::string studyuid, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);
	void DoSendSeriesAsync(std::string studyuid, std::string seriesuid, bool changeinfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, DestinationEntry destination);
	void DoQuickSendAsync(DestinationEntry destination);	
	static bool Echo(DestinationEntry destination);
	std::string ReadLog();

	void Cancel();
	bool IsDone();				
protected:
	DICOMSenderImpl *impl;
};

#endif