cmake_minimum_required(VERSION 3.19)
project(kwaslib)

set(KWASLIB_CORE_ONLY False CACHE STRING "Build only core functionality of kwaslib")

set(KWASLIB_BUILD_CRI True CACHE STRING "Build with CRIWARE")
set(KWASLIB_BUILD_HE True CACHE STRING "Build with Hedgehog Engine")
set(KWASLIB_BUILD_NW4R True CACHE STRING "Build with Nintendo Ware For Revolution")
set(KWASLIB_BUILD_PLATINUM True CACHE STRING "Build with Platinum Engine")

set(KWASLIB_CORE_SOURCES
	${PROJECT_SOURCE_DIR}/core/cpu/endianness.c
	${PROJECT_SOURCE_DIR}/core/crypto/crc32.c
	${PROJECT_SOURCE_DIR}/core/io/arg_parser.c
	${PROJECT_SOURCE_DIR}/core/io/dir_list.c
	${PROJECT_SOURCE_DIR}/core/io/file_utils.c
	${PROJECT_SOURCE_DIR}/core/io/path_utils.c
	${PROJECT_SOURCE_DIR}/core/io/type_readers.c
	${PROJECT_SOURCE_DIR}/core/io/type_writers.c
	${PROJECT_SOURCE_DIR}/core/math/boundary.c
	${PROJECT_SOURCE_DIR}/core/math/half.c
	${PROJECT_SOURCE_DIR}/core/math/vec.c
	${PROJECT_SOURCE_DIR}/core/data/dbl_link_list.c
	${PROJECT_SOURCE_DIR}/core/data/image/image.c
	${PROJECT_SOURCE_DIR}/core/data/image/s3tc.c
	${PROJECT_SOURCE_DIR}/core/data/image/gtf.c
	${PROJECT_SOURCE_DIR}/core/data/image/x360_texture.c
	)
	
set(KWASLIB_CRI_SOURCES
	${PROJECT_SOURCE_DIR}/cri/acb_command.c
	${PROJECT_SOURCE_DIR}/cri/utf.c
	${PROJECT_SOURCE_DIR}/cri/awb.c
	)
	
set(KWASLIB_HE_SOURCES
	${PROJECT_SOURCE_DIR}/he/BINA.c
	${PROJECT_SOURCE_DIR}/he/mirage.c
	${PROJECT_SOURCE_DIR}/he/uv_anim.c
	${PROJECT_SOURCE_DIR}/he/cam_anim.c
	)
	
set(KWASLIB_NW4R_SOURCES
	${PROJECT_SOURCE_DIR}/nw4r/brres.c
	${PROJECT_SOURCE_DIR}/nw4r/srt0.c
	${PROJECT_SOURCE_DIR}/nw4r/scn0.c
	)
	
set(KWASLIB_PLATINUM_SOURCES
	${PROJECT_SOURCE_DIR}/platinum/dat.c
	${PROJECT_SOURCE_DIR}/platinum/wmb4.c
	${PROJECT_SOURCE_DIR}/platinum/wtb.c
	)
	
set(KWASLIB_SOURCES
	${KWASLIB_CORE_SOURCES}
	)

# Adding individual requested modules
if(NOT ${KWASLIB_CORE_ONLY})
	if(${KWASLIB_BUILD_CRI})
		list(APPEND KWASLIB_SOURCES "${KWASLIB_CRI_SOURCES}")
	endif()

	if(${KWASLIB_BUILD_HE})
		list(APPEND KWASLIB_SOURCES "${KWASLIB_HE_SOURCES}")
	endif()

	if(${KWASLIB_BUILD_NW4R})
		list(APPEND KWASLIB_SOURCES "${KWASLIB_NW4R_SOURCES}")
	endif()

	if(${KWASLIB_BUILD_PLATINUM})
		list(APPEND KWASLIB_SOURCES "${KWASLIB_PLATINUM_SOURCES}")
	endif()
endif()

add_library(kwaslib SHARED ${KWASLIB_SOURCES})

target_include_directories(kwaslib PRIVATE "${PROJECT_SOURCE_DIR}/../")
