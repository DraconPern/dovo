#ifndef _DICOMSCANNER_
#define _DICOMSCANNER_

#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
#include "sqlite3.h"

class DICOMFileScannerImpl;
class DICOMFileScanner
{
public:
	DICOMFileScanner();
	~DICOMFileScanner();

	void Initialize(sqlite3 *db, boost::filesystem::path scanPath);
	void Clear(void);

	static void DoScanThread(void *obj);

	void Cancel();
	bool IsDone();
private:
	DICOMFileScannerImpl *impl;
};

#endif