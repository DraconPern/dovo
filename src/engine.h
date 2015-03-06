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
	void StartScan(wxString path);
	void StopScan();
	void StartSend(wxString PatientName, wxString NewPatientName, wxString NewPatientID, wxString NewBirthDay, int destination);
	void StopSend();
	void GetPatients(sqlite3_callback fillname, void *obj);
	DICOMFileScanner scanner;
	DICOMSender sender;
protected:

	sqlite3 *db;
	
};

#endif