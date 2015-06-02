
#include "dicomscanner.h"
#include "sqlite3_exec_stmt.h"
#include <codecvt>

// #include <locale>


// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dctk.h"

// check DCMTK functionality
#if !defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
#error "DCMTK and this program must be compiled with DCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS"
#endif

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif



class DICOMFileScannerImpl
{
public:
	DICOMFileScannerImpl(void);
	~DICOMFileScannerImpl(void);

	void Initialize(sqlite3 *db, boost::filesystem::path scanPath);

	void DoScan(boost::filesystem::path path);

	static void DoScanThread(void *obj);
	boost::filesystem::path m_scanPath;

	void Cancel();
	bool IsDone();

	sqlite3 *db;
protected:
	void ScanFile(boost::filesystem::path path);
	void ScanDir(boost::filesystem::path path);

	bool IsCanceled();
	void SetDone(bool state);

	boost::mutex mutex;
	bool cancelEvent, doneEvent;

	sqlite3_stmt *insertImage;
};

DICOMFileScannerImpl::DICOMFileScannerImpl()
{
	cancelEvent = doneEvent = false;
	db = NULL;
	m_scanPath = "";
}

DICOMFileScannerImpl::~DICOMFileScannerImpl()
{	
}

void DICOMFileScannerImpl::Initialize(sqlite3 *db, boost::filesystem::path scanPath)
{
	cancelEvent = doneEvent = false;
	this->db = db;
	this->m_scanPath = scanPath;
}

void DICOMFileScannerImpl::ScanFile(boost::filesystem::path path)
{
	DcmFileFormat dfile;
	OFCondition cond = dfile.loadFile(path.c_str());
	if (cond.good())
	{		
		OFString patientname, patientid, birthday;
		OFString studyuid, modality, studydesc, studydate;
		OFString seriesuid, seriesdesc;
		OFString sopuid;
		dfile.getDataset()->findAndGetOFString(DCM_PatientName, patientname);
		if(patientname.size() == 0)
			return;

		dfile.getDataset()->findAndGetOFString(DCM_PatientID, patientid);
		dfile.getDataset()->findAndGetOFString(DCM_PatientBirthDate, birthday);
		dfile.getDataset()->findAndGetOFString(DCM_StudyInstanceUID, studyuid);
		dfile.getDataset()->findAndGetOFString(DCM_Modality, modality);
		dfile.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc);
		dfile.getDataset()->findAndGetOFString(DCM_StudyDate, studydate);

		dfile.getDataset()->findAndGetOFString(DCM_SeriesInstanceUID, seriesuid);
		dfile.getDataset()->findAndGetOFString(DCM_SeriesDescription, seriesdesc);

		dfile.getDataset()->findAndGetOFString(DCM_SOPInstanceUID, sopuid);

		sqlite3_bind_text(insertImage, 1, patientname.c_str(), strlen(patientname.c_str()), SQLITE_STATIC);
		sqlite3_bind_text(insertImage, 2, patientid.c_str(), strlen(patientid.c_str()), SQLITE_STATIC);
		sqlite3_bind_text(insertImage, 3, birthday.c_str(), strlen(birthday.c_str()), SQLITE_STATIC);

		sqlite3_bind_text(insertImage, 4, studyuid.c_str(), strlen(studyuid.c_str()), SQLITE_STATIC);
		sqlite3_bind_text(insertImage, 5, modality.c_str(), strlen(modality.c_str()), SQLITE_STATIC);
		sqlite3_bind_text(insertImage, 6, studydesc.c_str(), strlen(studydesc.c_str()), SQLITE_STATIC);
		sqlite3_bind_text(insertImage, 7, studydate.c_str(), strlen(studydate.c_str()), SQLITE_STATIC);

		sqlite3_bind_text(insertImage, 8, seriesuid.c_str(), strlen(seriesuid.c_str()), SQLITE_STATIC);
		sqlite3_bind_text(insertImage, 9, seriesdesc.c_str(), strlen(seriesdesc.c_str()), SQLITE_STATIC);

		sqlite3_bind_text(insertImage, 10, sopuid.c_str(), strlen(sopuid.c_str()), SQLITE_STATIC);

#ifdef _WIN32
		std::string p = path.string(std::codecvt_utf8<boost::filesystem::path::value_type>());
		sqlite3_bind_text(insertImage, 11, p.c_str(), p.length(), SQLITE_STATIC);
#else
		sqlite3_bind_text(insertImage, 11, path.string().c_str(), path.string().length(), SQLITE_STATIC);
#endif
		sqlite3_exec_stmt(insertImage, NULL, NULL, NULL);

	}

}

void DICOMFileScannerImpl::DoScanThread(void *obj)
{
	DICOMFileScannerImpl *me = (DICOMFileScannerImpl *) obj;
	if(me)
	{
		me->SetDone(false);
		me->DoScan(me->m_scanPath);
		me->SetDone(true);
	}

}

void DICOMFileScannerImpl::DoScan(boost::filesystem::path path)
{	
	OFLog::configure(OFLogger::OFF_LOG_LEVEL);

	std::string imagesql = "INSERT INTO images VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0)";
	sqlite3_prepare_v2(db, imagesql.c_str(), imagesql.length(), &insertImage, NULL);

	ScanDir(path);

	sqlite3_finalize(insertImage);
}

void DICOMFileScannerImpl::ScanDir(boost::filesystem::path path)
{
	boost::filesystem::path someDir(path);
	boost::filesystem::directory_iterator end_iter;

	// catch any access errors
	try
	{
		if ( boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
		{
			for( boost::filesystem::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; dir_iter++)
			{
				if(IsCanceled())
				{			
					break;
				}

				if (boost::filesystem::is_regular_file(dir_iter->status()) )
				{
					ScanFile(*dir_iter);
				}
				else if (boost::filesystem::is_directory(dir_iter->status()) )
				{
					// descent recursively
					ScanDir(*dir_iter);
				}
			}

		}
	}
	catch(...)
	{

	}
}


void DICOMFileScannerImpl::Cancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = true;
}

bool DICOMFileScannerImpl::IsDone()
{
	boost::mutex::scoped_lock lk(mutex);
	return doneEvent;
}

bool DICOMFileScannerImpl::IsCanceled()
{
	boost::mutex::scoped_lock lk(mutex);
	return cancelEvent;
}

void DICOMFileScannerImpl::SetDone(bool state)
{
	boost::mutex::scoped_lock lk(mutex);
	doneEvent = state;
}

DICOMFileScanner::DICOMFileScanner(void)
{
	impl = new DICOMFileScannerImpl;
}

DICOMFileScanner::~DICOMFileScanner(void)
{
	delete impl;
}

void DICOMFileScanner::Initialize(sqlite3 *db, boost::filesystem::path scanPath)
{
	impl->Initialize(db, scanPath);
}

void DICOMFileScanner::DoScanThread(void *obj)
{	
	DICOMFileScannerImpl::DoScanThread(((DICOMFileScanner *) obj)->impl);
}


void DICOMFileScanner::Cancel()
{
	impl->Cancel();
}

bool DICOMFileScanner::IsDone()
{
	return impl->IsDone();
}
