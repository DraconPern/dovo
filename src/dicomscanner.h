#ifndef _DICOMSCANNER_
#define _DICOMSCANNER_

#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
#include "sqlite3.h"

class DICOMFileScanner
{
public:
	DICOMFileScanner(void);
	~DICOMFileScanner(void);

	void Clear(void);

	void DoScan(boost::filesystem::path path);

	static void DoScanThread(void *obj);
	boost::filesystem::path m_scanPath;

	void Cancel();
	bool IsDone();

	sqlite3 *db;
private:
	void ScanFile(boost::filesystem::path path);
	void ScanDir(boost::filesystem::path path);

	bool IsCanceled();
	void SetDone(bool state);

	boost::mutex mutex;
	bool cancelEvent, doneEvent;

	sqlite3_stmt *insertImage;
};

#endif