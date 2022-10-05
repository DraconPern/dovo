# dovo
Windows [![Build Status](https://home.draconpern.com:8081/buildStatus/icon?job=dovo.win.release)](https://home.draconpern.com:8081/job/dovo.win.release/) OS X [![Build Status](https://home.draconpern.com:8081/buildStatus/icon?job=dovo.osx.release)](https://home.draconpern.com:8081/job/dovo.osx.release/) Linux [![Build Status](https://home.draconpern.com:8081/buildStatus/icon?job=dovo.ubuntu.release)](https://home.draconpern.com:8081/job/dovo.ubuntu.release/)

Point of care, cross-platform software for importing DICOM CD and files then sending it to PACS. Usage scenario is front desk staff getting handed a CD with patient's images.  This tool allows the front desk to preview the images, and send to PACS.  Tested on Windows and OS X.

- Supports Group Policy for PACS destinations on Windows.
- Supports Windows Vista and above including Windows 10
- Supported on OS X
- Works on Linux
- Supports Unicode file and path.
- Image preview
- No dll's need to be distributed.
- Native, no Java required.

## Download
Binary https://www.draconpern.com/software/dovo

Source https://github.com/DraconPern/dovo

## Development notes
The program is http://utf8everywhere.org/
The data in sqlite is always utf8.
wxWidgets is utf32 on Windows and utf8 everywhere else.

## Requirements
- CMake http://www.cmake.org/download/
- XCode on OS X
- Visual Studio 2012 or higher on Windows (VS 2015 is used for release)
- gcc on Linux

## Third party dependency
The build scripts pull these automatically.
- wxWidgets http://www.wxwidgets.org/ please extract under ./wxWidgets
- DCMTK http://dicom.offis.de/ please use snapshot or git, and extract under ./dcmtk
- boost http://www.boost.org/ please extract under ./boost
- Visual Leak Detector https://vld.codeplex.com/ installed for debug release for Windows
- zlib please extract under ./zlib on Windows
- openjpeg http://www.openjpeg.org please extract under ./openjpeg
- fmjpeg2koj https://github.com/DraconPern/fmjpeg2koj please extract under ./fmjpeg2koj

## Author
Ing-Long Eric Kuo <draconpern@hotmail.com>

Follow https://twitter.com/DraconPernDev

## License
This software is licensed under the GPL.  For use under another license, please contact Ing-Long Eric Kuo <eric@frontmotion.com>
