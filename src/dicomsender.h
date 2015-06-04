#ifndef _DICOMSENDER_
#define _DICOMSENDER_

#include <boost/filesystem.hpp>
#include "sqlite3.h"

class DICOMSenderImpl;
class DICOMSender
{
public:
	DICOMSender();
	~DICOMSender();

	void Initialize(sqlite3 *db, const std::string PatientName, std::string PatientID ,std::string BirthDay,
		std::string NewPatientName, std::string NewPatientID ,std::string NewBirthDay,
		std::string destinationHost, unsigned int destinationPort, std::string destinationAETitle, std::string ourAETitle);

	static void DoSendThread(void *obj);

	std::string ReadLog();

	void Cancel();
	bool IsDone();	
private:
	DICOMSenderImpl *impl;
};

#endif