cmake_minimum_required(VERSION 3.19)
project(kwaslib)

set(KWASLIB_SOURCES
	${PROJECT_SOURCE_DIR}/utils/cpu/endianness.c>
	${PROJECT_SOURCE_DIR}/utils/crypto/crc32.c>
	${PROJECT_SOURCE_DIR}/utils/io/arg_parser.c>
	${PROJECT_SOURCE_DIR}/utils/io/dir_list.c>
	${PROJECT_SOURCE_DIR}/utils/io/file_utils.c>
	${PROJECT_SOURCE_DIR}/utils/io/path_utils.c>
	${PROJECT_SOURCE_DIR}/utils/io/type_readers.c>
	${PROJECT_SOURCE_DIR}/utils/io/type_writers.c>
	${PROJECT_SOURCE_DIR}/utils/math/boundary.c>
	${PROJECT_SOURCE_DIR}/utils/math/half.c>
	${PROJECT_SOURCE_DIR}/utils/math/vec.c>
	
	${PROJECT_SOURCE_DIR}/he/BINA.c
	
	${PROJECT_SOURCE_DIR}/platinum/dat.c
	${PROJECT_SOURCE_DIR}/platinum/wmb4.c
	${PROJECT_SOURCE_DIR}/platinum/wtb.c
	)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -Qn -fno-ident -fPIC" CACHE STRING "Compiler flags" FORCE)
	
add_library(kwaslib SHARED ${KWASLIB_SOURCES})

target_include_directories(kwaslib PRIVATE "../")

