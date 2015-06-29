#ifndef _ENGINE_
#define _ENGINE_
#include <vector>
#include "sqlite3.h"
#include "destinationentry.h"
#include "dicomscanner.h"
#include "dicomsender.h"

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
	void StartSend(std::string PatientName, std::string PatientID, std::string BirthDay, std::string NewPatientName, std::string NewPatientID, std::string NewBirthDay, int destination);
	void StopSend();
	void GetPatients(sqlite3_callback fillname, void *obj);
	void GetStudies(std::string patientid, sqlite3_callback fillstudy, void *obj);
	void GetSeries(std::string studyuid, sqlite3_callback fillseries, void *obj);
	void GetInstances(std::string seriesuid, sqlite3_callback fillinstances, void *obj);
	void GetImage(std::string sopid, sqlite3_callback fillinstances, void *obj);
	DICOMFileScanner scanner;
	DICOMSender sender;
protected:

	sqlite3 *db;
	
};

#endif