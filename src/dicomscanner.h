#pragma once
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

	void DoScan(std::wstring path);

	static void DoScanThread(void *obj);
	std::wstring ScanPath;

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

	sqlite3_stmt *insertPatient, *insertStudy, *insertSeries, *insertImage;
};

