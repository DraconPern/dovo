# dovo
Cross-platform software for Importing DICOM files and sending to PACS. Tested on Windows and OS X.

- Supports Group Policy for PACS destinations on Windows.
- 32bit on Windows, 64bit on OS X.
- Supports Unicode file and path.
- Image preview (coming soon)
- No dll's need to be distributed.

## Development notes
The program is http://utf8everywhere.org/

## Requirements
- CMake http://www.cmake.org/download/

## Third party dependency
- wxWidgets http://www.wxwidgets.org/ please extract under ./wxWidgets
- DCMTK http://dicom.offis.de/ please use snapshot or git, and extract under ./dcmtk
- boost http://www.boost.org/ please extract under ./boost
 