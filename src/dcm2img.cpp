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
#include "dcmtk/dcmimage/diregist.h" // register support for color

#include "dcmtk/dcmdata/dcrledrg.h"	// rle decoder
#include "dcmtk/dcmjpeg/djdecode.h"	// jpeg decoder
// check DCMTK functionality
#if !defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
#error "DCMTK and this program must be compiled with DCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS"
#endif

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif

#include <wx/image.h>
#include <wx/rawbmp.h>

void dcm2img(boost::filesystem::path filename, int clientWidth, int clientHeight, wxImage &image)
{
	OFLog::configure(OFLogger::OFF_LOG_LEVEL);

	DcmRLEDecoderRegistration::registerCodecs();
	DJDecoderRegistration::registerCodecs();	
	
	DicomImage *small = NULL;
	char *source = NULL;

	try
	{
		DcmFileFormat dfile;
		if (dfile.loadFile(filename.c_str(), EXS_Unknown, EGL_withoutGL).bad())
			throw std::runtime_error("");
		if(dfile.loadAllDataIntoMemory().bad())
			throw std::runtime_error("");

		E_TransferSyntax xfer = EXS_Unknown; //dfile.getDataset()->getOriginalXfer();
		DicomImage di(&dfile, xfer, CIF_MayDetachPixelData, 0, 1);
		
		if(di.getStatus() != EIS_Normal)
			throw std::runtime_error("");

		di.getFirstFrame();
		di.showAllOverlays();			
		if (di.isMonochrome())
		{
			if(di.getWindowCount())
				di.setWindow(0);
			else
				di.setHistogramWindow();
		}
				
		small = di.createScaledImage((unsigned long) clientWidth, 0, 3, 0);
		if(small == NULL)
			throw std::runtime_error("");
				
		unsigned int width = small->getWidth();
		unsigned int height = small->getHeight();

		if(!image.Create(width, height, true))
			throw std::runtime_error("");
		
		source = new char[width * height * 4];
		char *source2 = source;

		small->createWindowsDIB((void * &)source, width * height * 4, 0, 32);			
		
		wxImagePixelData data(image);
		wxImagePixelData::Iterator p(data);
		for(int j = 0; j < height; j++)
		{			
			for(int i = 0; i < width; i++)
			{
				p.MoveTo(data, i, j);
				p.Red() = source[(width * j + i) * 4 + 2];
				p.Green() = source[(width * j + i) * 4 + 1];
				p.Blue() = source[(width * j + i) * 4];
			}
		}		
	}
	catch(...)
	{

	}

	if(source != NULL)
		delete source;

	if(small != NULL)
		delete small;

	DJDecoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();
}