
#include "dicomscanner.h"

#include <boost/thread.hpp>

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


DICOMFileScanner::DICOMFileScanner(PatientData &patientdata) 
	: patientdata(patientdata)
{
	cancelEvent = doneEvent = false;	
	m_scanPath = "";
}

DICOMFileScanner::~DICOMFileScanner()
{	
}

void DICOMFileScanner::ScanFile(boost::filesystem::path path)
{
	std::string extension = path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (!extension.empty() && extension != "dcm")
	{
		if (extension == "exe" ||
			extension == "dll" ||
			extension == "jpg" ||
			extension == "bmp" ||
			extension == "mdb" ||
			extension == "zip")
			return;
	}

	std::string filename = path.filename().string();
	if (filename == "DICOMDIR")
		return;

	DcmFileFormat dfile;
	OFCondition cond = dfile.loadFileUntilTag(path.c_str(), EXS_Unknown, EGL_noChange, 4096UL, ERM_autoDetect, DCM_InstanceNumber);
	if (cond.good())
	{
		OFString patientname, patientid, birthday;
		OFString studyuid, modality, studydesc, studydate;
		OFString seriesuid, seriesdesc;
		OFString sopuid, sopclassuid, transfersyntax;

		dfile.getDataset()->convertToUTF8();

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
		dfile.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopclassuid);
	
		DcmXfer filexfer(dfile.getDataset()->getOriginalXfer());
		transfersyntax = filexfer.getXferID();
		
		patientdata.AddPatient(patientid.c_str(), patientname.c_str(), birthday.c_str());
		patientdata.AddStudy(studyuid.c_str(), patientid.c_str(), patientname.c_str(), studydesc.c_str(), studydate.c_str());
		patientdata.AddSeries(seriesuid.c_str(), studyuid.c_str(), seriesdesc.c_str());
	
		patientdata.AddInstance(sopuid.c_str(), seriesuid.c_str(), path, sopclassuid.c_str(), transfersyntax.c_str());		
	}

}

void DICOMFileScanner::DoScanAsync(boost::filesystem::path path)
{
	SetDone(false);
	ClearCancel();

	m_scanPath = path;
	boost::thread t(DICOMFileScanner::DoScanThread, this);
	t.detach();	
}

void DICOMFileScanner::DoScanThread(void *obj)
{
	DICOMFileScanner *me = (DICOMFileScanner *) obj;
	if(me)
	{
		me->DoScan(me->m_scanPath);
		me->SetDone(true);
	}

}

void DICOMFileScanner::DoScan(boost::filesystem::path path)
{	
	OFLog::configure(OFLogger::OFF_LOG_LEVEL);
	
	// catch any access errors
	try
	{
		ScanDir(path);
	}
	catch(...)
	{

	}
	
}

void DICOMFileScanner::ScanDir(boost::filesystem::path path)
{
	std::string filename = path.filename().string();
	std::transform(filename.begin(), filename.end(), filename.begin(),
		[](unsigned char c) { return std::tolower(c); });

	//if (filename == "exe")
	//	return;

	boost::filesystem::directory_iterator end_iter;
	if (boost::filesystem::is_directory(path))
	{
		for( boost::filesystem::directory_iterator dir_iter(path) ; dir_iter != end_iter ; dir_iter++)
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

void DICOMFileScanner::Cancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = true;
}

void DICOMFileScanner::ClearCancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = false;
}

bool DICOMFileScanner::IsDone()
{
	boost::mutex::scoped_lock lk(mutex);
	return doneEvent;
}

bool DICOMFileScanner::IsCanceled()
{
	boost::mutex::scoped_lock lk(mutex);
	return cancelEvent;
}

void DICOMFileScanner::SetDone(bool state)
{
	boost::mutex::scoped_lock lk(mutex);
	doneEvent = state;
}
