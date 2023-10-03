cmake_minimum_required(VERSION 3.19)
project(kwaslib)

set(KWASLIB_CORE_ONLY False CACHE STRING "Build only core functionality of kwaslib")

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
	)

set(KWASLIB_MODULES_SOURCES
	${PROJECT_SOURCE_DIR}/he/BINA.c
	${PROJECT_SOURCE_DIR}/he/mirage.c
	${PROJECT_SOURCE_DIR}/he/uv_anim.c
	${PROJECT_SOURCE_DIR}/he/cam_anim.c

	${PROJECT_SOURCE_DIR}/cri/utf.c
	${PROJECT_SOURCE_DIR}/cri/awb.c

	${PROJECT_SOURCE_DIR}/platinum/dat.c
	${PROJECT_SOURCE_DIR}/platinum/wmb4.c
	${PROJECT_SOURCE_DIR}/platinum/wtb.c
	)

set(KWASLIB_SOURCES
	${KWASLIB_CORE_SOURCES}
	)

if(NOT ${KWASLIB_CORE_ONLY})
	list(APPEND KWASLIB_SOURCES "${KWASLIB_MODULES_SOURCES}")
endif()

add_library(kwaslib SHARED ${KWASLIB_SOURCES})

target_include_directories(kwaslib PRIVATE "${PROJECT_SOURCE_DIR}/../")
