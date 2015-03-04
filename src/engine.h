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
protected:

	sqlite3 *db;
	DICOMFileScanner scanner;


};