#ifndef _DICOMSCANNER_
#define _DICOMSCANNER_

#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include "patientdata.h"


class DICOMFileScanner
{
public:
	DICOMFileScanner(PatientData &patientdata);
	~DICOMFileScanner(void);
	
	void DoScanAsync(boost::filesystem::path path);
	void DoScan(boost::filesystem::path path);		

	void Cancel();
	bool IsDone();

	PatientData &patientdata;
protected:
	static void DoScanThread(void *obj);
	void ScanFile(boost::filesystem::path path);
	void ScanDir(boost::filesystem::path path);

	bool IsCanceled();
	void ClearCancel();
	void SetDone(bool state);

	// threading data
	boost::mutex mutex;
	bool cancelEvent, doneEvent;	
	boost::filesystem::path m_scanPath;
};
#endif