cmake_minimum_required(VERSION 3.19)
project(kwaslib)

set(KWASLIB_CORE_ONLY False CACHE STRING "Build only core functionality of kwaslib")

set(KWASLIB_BUILD_CRI True CACHE STRING "Build with CRIWARE")
set(KWASLIB_BUILD_HE True CACHE STRING "Build with Hedgehog Engine")
set(KWASLIB_BUILD_NW4R True CACHE STRING "Build with Nintendo Ware For Revolution")
set(KWASLIB_BUILD_PLATINUM True CACHE STRING "Build with Platinum Engine")

set(KWASLIB_CORE_SOURCES
	#${PROJECT_SOURCE_DIR}/core/cpu/endianness.c
	
	${PROJECT_SOURCE_DIR}/core/crypto/crc32.c
	
	${PROJECT_SOURCE_DIR}/core/io/arg_parser.c
	${PROJECT_SOURCE_DIR}/core/io/dir_list.c
	${PROJECT_SOURCE_DIR}/core/io/file_utils.c
	${PROJECT_SOURCE_DIR}/core/io/path_utils.c
	#${PROJECT_SOURCE_DIR}/core/io/type_readers.c
	#${PROJECT_SOURCE_DIR}/core/io/type_writers.c
	${PROJECT_SOURCE_DIR}/core/io/string_utils.c
    
	${PROJECT_SOURCE_DIR}/core/math/boundary.c
	${PROJECT_SOURCE_DIR}/core/math/half.c
	${PROJECT_SOURCE_DIR}/core/math/vec.c

	${PROJECT_SOURCE_DIR}/core/data/dbl_link_list.c
	${PROJECT_SOURCE_DIR}/core/data/cvector.c
	${PROJECT_SOURCE_DIR}/core/data/vl.c

	${PROJECT_SOURCE_DIR}/core/data/image/dds.c
	${PROJECT_SOURCE_DIR}/core/data/image/image.c
	${PROJECT_SOURCE_DIR}/core/data/image/s3tc.c
	${PROJECT_SOURCE_DIR}/core/data/image/gtf.c
	${PROJECT_SOURCE_DIR}/core/data/image/x360_texture.c
    
	${PROJECT_SOURCE_DIR}/core/data/text/sexml.c
	${PROJECT_SOURCE_DIR}/core/data/text/sexml_exporter.c
	${PROJECT_SOURCE_DIR}/core/data/text/sexml_io.c
	${PROJECT_SOURCE_DIR}/core/data/text/sexml_parser.c
	)
	
set(KWASLIB_CRI_SOURCES
	${PROJECT_SOURCE_DIR}/cri/acb/acb_command.c
	${PROJECT_SOURCE_DIR}/cri/archive/afs.c
	${PROJECT_SOURCE_DIR}/cri/archive/afs_parse.c
	${PROJECT_SOURCE_DIR}/cri/archive/afs_export.c
	${PROJECT_SOURCE_DIR}/cri/audio/adx.c
	${PROJECT_SOURCE_DIR}/cri/audio/awb.c
	${PROJECT_SOURCE_DIR}/cri/audio/hca.c
	${PROJECT_SOURCE_DIR}/cri/utf/utf.c
	${PROJECT_SOURCE_DIR}/cri/utf/utf_common.c
	${PROJECT_SOURCE_DIR}/cri/utf/utf_data_table.c
	${PROJECT_SOURCE_DIR}/cri/utf/utf_load.c
	${PROJECT_SOURCE_DIR}/cri/utf/utf_save.c
	${PROJECT_SOURCE_DIR}/cri/utf/utf_string_table.c
	${PROJECT_SOURCE_DIR}/cri/utf/utf_table.c
	)
	
set(KWASLIB_HE_SOURCES
	${PROJECT_SOURCE_DIR}/he/BINA.c
	${PROJECT_SOURCE_DIR}/he/mirage.c
	${PROJECT_SOURCE_DIR}/he/mirage_keyframe.c
	${PROJECT_SOURCE_DIR}/he/mirage_string_table.c
	${PROJECT_SOURCE_DIR}/he/cam_anim.c
	${PROJECT_SOURCE_DIR}/he/mat_anim.c
	${PROJECT_SOURCE_DIR}/he/morph_anim.c
	${PROJECT_SOURCE_DIR}/he/pt_anim.c
	${PROJECT_SOURCE_DIR}/he/uv_anim.c
	${PROJECT_SOURCE_DIR}/he/vis_anim.c
	${PROJECT_SOURCE_DIR}/he/lit_anim.c
	)
	
set(KWASLIB_NW4R_SOURCES
	${PROJECT_SOURCE_DIR}/nw4r/brres.c
	${PROJECT_SOURCE_DIR}/nw4r/srt0.c
	${PROJECT_SOURCE_DIR}/nw4r/scn0.c
	)
	
set(KWASLIB_PLATINUM_SOURCES
	${PROJECT_SOURCE_DIR}/platinum/dat.c
	${PROJECT_SOURCE_DIR}/platinum/dat_hashtable.c
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

# Test for Big Endian
include(TestBigEndian)
TEST_BIG_ENDIAN(KWASLIB_IS_BIG_ENDIAN)
if(KWASLIB_IS_BIG_ENDIAN)
    add_definitions(-DKWASLIB_BIG_ENDIAN)
else()
    add_definitions(-DKWASLIB_LITTLE_ENDIAN)
endif()

add_library(kwaslib SHARED ${KWASLIB_SOURCES})

target_include_directories(kwaslib PRIVATE "${PROJECT_SOURCE_DIR}/../")
