cmake_minimum_required(VERSION 2.8)

project(dovo)

set(VERSION "1.0.0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
#set(Boost_USE_STATIC_RUNTIME     OFF)
find_package(Boost REQUIRED thread filesystem)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${DCMJPEG2000}/include )
#LINK_DIRECTORIES(${DCMTK_ROOT}/lib ${ZLIB_LIBDIR} ${Boost_LIBRARY_DIRS} ${DCMJPEG2000}/lib ${IMAGEMAGICK}/VisualMagick/lib ${ULTIMATETOOLBOX}/lib/Libs)

set(dovo_HEADERS	
	src/gui.h
	src/dovo_about.h
	src/dovo_changePatientInfo.h
	src/dovo_destination.h
	src/dovo_mainFrame.h
	src/destinationentry.h
)

set(dovo_SRCS
    ${dovo_HEADERS}    
	src/main.cpp
	src/gui.cpp
	src/dovo_about.cpp
	src/dovo_changePatientInfo.cpp
	src/dovo_destination.cpp
	src/dovo_mainFrame.cpp
)

#VC++ stuff
add_definitions(-D_UNICODE)
add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)

ADD_EXECUTABLE(dovo WIN32 ${dovo_SRCS})

IF(WIN32)
  SET(OS_SPECIFIC_LIBS netapi32 wsock32 odbc32 Rpcrt4)
ELSE(WIN32)
  SET(OS_SPECIFIC_LIBS )
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(dovo
	
	${wxWidgets_LIBRARIES}
	${OS_SPECIFIC_LIBS}
	)

	
