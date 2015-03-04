#ifndef _ENGINE_
#define _ENGINE_
#include <vector>
#include "sqlite3.h"
#include "destinationentry.h"
#include "dicomscanner.h"

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
	void GetPatients(sqlite3_callback fillname, void *obj);
	DICOMFileScanner scanner;
protected:

	sqlite3 *db;
	
};

#endif