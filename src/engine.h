#include <vector>
#include "sqlite3.h"
#include "sqlite3_exec_stmt.h"

#include "destinationentry.h"

class engine
{
public:
	engine();
	~engine();

	std::vector<DestinationEntry> destinations, globalDestinations;
	void LoadDestinationList();
	void LoadGlobalDestinationList();
	void SaveDestinationList();
	void update();
protected:
	sqlite3 *db;



};