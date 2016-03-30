cmake_minimum_required(VERSION 2.4)

project(jasper)

set(VERSION "1.0.1")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

string(REPLACE "\\" "/" JASPERDIR ${JASPERDIR})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${JASPERDIR}/src/libjasper/include )
LINK_DIRECTORIES(${ZLIB_LIBDIR})

set(jasper_HEADERS
	${JASPERDIR}/src/libjasper/include/jasper/jasper.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_cm.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_config.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_config2.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_debug.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_fix.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_getopt.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_icc.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_image.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_init.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_malloc.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_math.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_seq.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_stream.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_string.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_tmr.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_tvp.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_types.h
	${JASPERDIR}/src/libjasper/include/jasper/jas_version.h
)

set(jasper_SRCS
    ${fmjpeg2k_HEADERS}    
	${JASPERDIR}/src/libjasper/bmp/bmp_cod.c
	${JASPERDIR}/src/libjasper/bmp/bmp_dec.c
	${JASPERDIR}/src/libjasper/bmp/bmp_enc.c
	${JASPERDIR}/src/libjasper/base/jas_cm.c
	${JASPERDIR}/src/libjasper/base/jas_debug.c
	${JASPERDIR}/src/libjasper/base/jas_getopt.c
	${JASPERDIR}/src/libjasper/base/jas_icc.c
	${JASPERDIR}/src/libjasper/base/jas_iccdata.c
	${JASPERDIR}/src/libjasper/base/jas_image.c
	${JASPERDIR}/src/libjasper/base/jas_init.c
	${JASPERDIR}/src/libjasper/base/jas_malloc.c
	${JASPERDIR}/src/libjasper/base/jas_seq.c
	${JASPERDIR}/src/libjasper/base/jas_stream.c
	${JASPERDIR}/src/libjasper/base/jas_string.c
	${JASPERDIR}/src/libjasper/base/jas_tmr.c
	${JASPERDIR}/src/libjasper/base/jas_tvp.c
	${JASPERDIR}/src/libjasper/base/jas_version.c
	${JASPERDIR}/src/libjasper/jp2/jp2_cod.c
	${JASPERDIR}/src/libjasper/jp2/jp2_dec.c
	${JASPERDIR}/src/libjasper/jp2/jp2_enc.c
	${JASPERDIR}/src/libjasper/jpc/jpc_bs.c
	${JASPERDIR}/src/libjasper/jpc/jpc_cs.c
	${JASPERDIR}/src/libjasper/jpc/jpc_dec.c
	${JASPERDIR}/src/libjasper/jpc/jpc_enc.c
	${JASPERDIR}/src/libjasper/jpc/jpc_math.c
	${JASPERDIR}/src/libjasper/jpc/jpc_mct.c
	${JASPERDIR}/src/libjasper/jpc/jpc_mqcod.c
	${JASPERDIR}/src/libjasper/jpc/jpc_mqdec.c
	${JASPERDIR}/src/libjasper/jpc/jpc_mqenc.c
	${JASPERDIR}/src/libjasper/jpc/jpc_qmfb.c
	${JASPERDIR}/src/libjasper/jpc/jpc_t1cod.c
	${JASPERDIR}/src/libjasper/jpc/jpc_t1dec.c
	${JASPERDIR}/src/libjasper/jpc/jpc_t1enc.c
	${JASPERDIR}/src/libjasper/jpc/jpc_t2cod.c
	${JASPERDIR}/src/libjasper/jpc/jpc_t2dec.c
	${JASPERDIR}/src/libjasper/jpc/jpc_t2enc.c
	${JASPERDIR}/src/libjasper/jpc/jpc_tagtree.c
	${JASPERDIR}/src/libjasper/jpc/jpc_tsfb.c
	${JASPERDIR}/src/libjasper/jpc/jpc_util.c
	${JASPERDIR}/src/libjasper/jpg/jpg_dummy.c
	${JASPERDIR}/src/libjasper/jpg/jpg_val.c
	${JASPERDIR}/src/libjasper/mif/mif_cod.c
	${JASPERDIR}/src/libjasper/pgx/pgx_cod.c
	${JASPERDIR}/src/libjasper/pgx/pgx_dec.c
	${JASPERDIR}/src/libjasper/pgx/pgx_enc.c
	${JASPERDIR}/src/libjasper/pnm/pnm_cod.c
	${JASPERDIR}/src/libjasper/pnm/pnm_dec.c
	${JASPERDIR}/src/libjasper/pnm/pnm_enc.c
	${JASPERDIR}/src/libjasper/ras/ras_cod.c
	${JASPERDIR}/src/libjasper/ras/ras_dec.c
	${JASPERDIR}/src/libjasper/ras/ras_enc.c
)


#VC++ stuff
if(WIN32)
add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)
add_definitions(-DJAS_WIN_MSVC_BUILD)
endif()

add_definitions(-DEXCLUDE_MIF_SUPPORT)
add_definitions(-DEXCLUDE_PNM_SUPPORT)
add_definitions(-DEXCLUDE_BMP_SUPPORT)
add_definitions(-DEXCLUDE_RAS_SUPPORT)
add_definitions(-DEXCLUDE_JPG_SUPPORT)
add_definitions(-DEXCLUDE_PGX_SUPPORT)

add_library(jasper ${jasper_SRCS})


TARGET_LINK_LIBRARIES(jasper
	)

install(TARGETS jasper
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib	  
	)
 
install(FILES ${jasper_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/jasper)
