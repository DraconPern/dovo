#include <boost/filesystem.hpp>

// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"

#include "dcmtk/dcmdata/dcrledrg.h"
#include "dcmtk/dcmjpeg/djdecode.h"
// check DCMTK functionality
#if !defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
#error "DCMTK and this program must be compiled with DCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS"
#endif

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif

#include <wx/image.h>

void dcm2img(boost::filesystem::path filename, wxImage &image)
{
	/*
	DcmRLEDecoderRegistration::registerCodecs();
	DJDecoderRegistration::registerCodecs();	
	
	DcmFileFormat dfile;	
	if (dfile.loadFile(filename.c_str(), EXS_Unknown, EGL_withoutGL).bad())
		return;
	if(dfile.loadAllDataIntoMemory().bad())
		return;
	
	E_TransferSyntax xfer = EXS_Unknown; //dfile.getDataset()->getOriginalXfer();
	DicomImage di(&dfile, xfer, CIF_MayDetachPixelData, 0, 1);

	if(di.getStatus() != EIS_Normal)
		return;

	di.getFirstFrame();
	di.showAllOverlays();			
	if (di.isMonochrome())
	{
		if(di.getWindowCount())
			di.setWindow(0);
		else
			di.setHistogramWindow();
	}

	image.Create(di.getWidth(), di.getHeight());	
	di.getOutputData(image.GetData(), di.getWidth() * di.getHeight() * 3, 24);	
	
	DJDecoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();	 */
}