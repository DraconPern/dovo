# dovo
Point of care, cross-platform software for importing DICOM CD and files then sending it to PACS. Usage scenario is front desk staff getting handed a CD with patient's images.  This tool allows the front desk to preview the images, and send to PACS.  Tested on Windows and OS X.

- Supports Group Policy for PACS destinations on Windows.
- 32bit on Windows, 64bit on OS X.
- Supports Unicode file and path.
- Image preview (coming soon)
- No dll's need to be distributed.

## Download
Binary http://www.draconpern.com/software/dovo
Source https://github.com/DraconPern/dovo

## Development notes
The program is http://utf8everywhere.org/

## Requirements
- CMake http://www.cmake.org/download/
- XCode on OS X
- Visual Studio 2012 or higher on Windows
- gcc on Linux

## Third party dependency
- wxWidgets http://www.wxwidgets.org/ please extract under ./wxWidgets
- DCMTK http://dicom.offis.de/ please use snapshot or git, and extract under ./dcmtk
- boost http://www.boost.org/ please extract under ./boost
 
