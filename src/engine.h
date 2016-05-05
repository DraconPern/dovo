#ifndef _ENGINE_
#define _ENGINE_
#include <vector>
#include "sqlite3.h"
#include "destinationentry.h"
#include "dicomscanner.h"
#include "dicomsender.h"
#include "patientdata.h"

class engine
{
public:
	engine();
	~engine();

	std::vector<DestinationEntry> destinations, globalDestinations;
	void LoadDestinationList();
	void LoadGlobalDestinationList();
	void SaveDestinationList();
	void StartScan(boost::filesystem::path path);
	void StopScan();	
	void StartSend(std::string PatientID, std::string PatientName, bool changePatientInfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, int destination);
	void StopSend();

	DICOMFileScanner scanner;
	DICOMSender sender;
	PatientData patientdata;

};

#endif