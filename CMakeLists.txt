cmake_minimum_required(VERSION 3.31)

project(dovo VERSION 1.0.20)

add_definitions(-DDOVO_VERSION="${PROJECT_VERSION}")

if(WIN32 OR APPLE)
set(CMAKE_CXX_STANDARD 11)
else()
#ubuntu 22.04
set(CMAKE_CXX_STANDARD 17)
endif()

if(WIN32)
  #VC++ stuff
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-D_UNICODE)
  add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
if(WIN32 OR APPLE)
  set(Boost_USE_STATIC_RUNTIME     ON)
endif()
find_package(Boost REQUIRED COMPONENTS headers thread filesystem system regex date_time locale)

if(WIN32)
  #enable dcmtk filename to have wide version, only affects WIN32.  http://utf8everywhere.org/
  add_definitions(-DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/dcmtk")
find_package(DCMTK REQUIRED CONFIG)

#include zlib
set(ZLIB_USE_STATIC_LIBS ON)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/zlib")
find_package(ZLIB REQUIRED)

#include openjpeg
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/openjpeg/Release/lib/openjpeg-2.4")
find_package(OpenJPEG REQUIRED)
add_definitions(-DOPJ_STATIC)

#include fmjpeg2koj
find_package(fmjpeg2k REQUIRED)

if(LINUX)
  find_package(OpenSSL CONFIG)
endif()

set(wxWidgets_USE_STATIC   ON)
#define this so that wxWidgets doesn't define it. Conflicts w/ dcmtk's declaration
#add_definitions(-DHAVE_SSIZE_T)
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base adv)

set(dovo_HEADERS
  src/gui.h
  src/dovo_about.h
  src/dovo_changePatientInfo.h
  src/dovo_destination.h
  src/dovo_mainFrame.h
  src/dovo_searchStatus.h
  src/dovo_sendStatus.h
  src/dovo_changePatientInfo.h
  src/dovo_updateCheck.h
  sqlite3/sqlite3.h
  src/engine.h
  src/patientdata.h
  src/destinationentry.h
  src/dicomscanner.h
  src/dicomsender.h
  src/dcm2img.h
  src/update.h
  src/alphanum.hpp
  src/fetch.h
)

set(dovo_SRCS
  ${dovo_HEADERS}
  src/main.cpp
  src/gui.cpp
  src/dovo_about.cpp
  src/dovo_changePatientInfo.cpp
  src/dovo_destination.cpp
  src/dovo_mainFrame.cpp
  src/dovo_searchStatus.cpp
  src/dovo_sendStatus.cpp
  src/dovo_changePatientInfo.cpp
  src/dovo_updateCheck.cpp
  sqlite3/sqlite3.c
  sqlite3/sqlite3_exec_stmt.c
  src/engine.cpp
  src/patientdata.cpp
  src/dicomscanner.cpp
  src/dicomsender.cpp
  src/dcm2img.cpp
  src/update.cpp
  src/fetch.cpp
)

if(WIN32)
  set(dovo_SRCS ${dovo_SRCS} src/win/dovo.rc src/win/version.rc src/win/dovo.manifest)

  string(REPLACE "." "," RC_DOVO_VERSION ${dovo_VERSION})

  add_definitions(-DRC_DOVO_VERSION=${RC_DOVO_VERSION})
endif()

if(APPLE)
  set(dovo_SRCS ${dovo_SRCS} src/osx/dovo.icns src/osx/Info.plist.in)
endif()

add_executable(dovo WIN32 MACOSX_BUNDLE ${dovo_SRCS})

target_include_directories(dovo PRIVATE ${CMAKE_SOURCE_DIR}/sqlite3 ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/boost-wintls/include ${FMJPEG2K_INCLUDE_DIRS} ${DCMTK_INCLUDE_DIRS})

if(WIN32)
  set(OS_SPECIFIC_LIBS netapi32 wsock32 odbc32 Rpcrt4 Crypt32)
else()
  if(APPLE)
  else()
    SET(OS_SPECIFIC_LIBS dl crypto ssl)
  endif()
endif()

if(APPLE)
  set_target_properties(dovo PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/src/osx/Info.plist.in)
  set_target_properties(dovo PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/src/osx/dovo.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

target_link_libraries(dovo
  ${FMJPEG2K_LIBRARIES}
  ${OPENJPEG_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${LIBICONV_LIBRARIES}
  DCMTK::DCMTK
  ${wxWidgets_LIBRARIES}
  Boost::thread
  Boost::filesystem
  Boost::system
  Boost::regex
  Boost::date_time
  Boost::locale	
  ${OS_SPECIFIC_LIBS}
  )

configure_file("installers/wix/Product.wxs.in" "${CMAKE_CURRENT_SOURCE_DIR}/installers/wix/Product.wxs")
configure_file("installers/deb/control.in" "${CMAKE_CURRENT_SOURCE_DIR}/installers/deb/control")
