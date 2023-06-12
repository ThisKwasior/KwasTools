cmake_minimum_required(VERSION 3.19)
project(kwaslib)

set(KWASLIB_SOURCES
	./utils/arg_parser.c
	./utils/dir_list.c
	./utils/endianness.c
	./utils/file_utils.c
	./utils/path_utils.c
	./utils/type_readers.c
	./utils/type_writers.c
	./he/BINA.c
	)

set(CMAKE_C_FLAGS "-s -Qn -fno-ident -fPIC" CACHE STRING "Compiler flags" FORCE)
	
add_library(kwaslib STATIC ${KWASLIB_SOURCES})

target_include_directories(kwaslib PRIVATE "../")

