cmake_minimum_required(VERSION 3.21)
project(KwasTools)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Configuration type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -fdata-sections -ffunction-sections" CACHE STRING "C Compiler flags" FORCE)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} " CACHE STRING "CXX Compiler flags" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm -fPIC -fno-ident -s -Wl,--gc-sections" CACHE STRING "EXE link flags" FORCE)
	
	set(CMAKE_STATIC_LINKER_FLAGS "-s" CACHE STRING "Static library linker flags" FORCE)
	set(CMAKE_SHARED_LINKER_FLAGS "-s" CACHE STRING "Shared library linker flags" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -Wall " CACHE STRING "C Compiler flags" FORCE)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} " CACHE STRING "CXX Compiler flags" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm " CACHE STRING "EXE link flags" FORCE)
endif()

set(KWASTOOLS_PROGRAMS
	${PROJECT_SOURCE_DIR}/src/cri_utf_tool.c
	${PROJECT_SOURCE_DIR}/src/cri_awb_tool.c
	
	#${PROJECT_SOURCE_DIR}/src/pcmodeltool.cpp
	${PROJECT_SOURCE_DIR}/src/he_anim_tool.c
	
	${PROJECT_SOURCE_DIR}/src/nw4r_misc_to_he_xml.c

	${PROJECT_SOURCE_DIR}/src/platinum_dat_tool.c
	${PROJECT_SOURCE_DIR}/src/platinum_wtb_tool.c
	#${PROJECT_SOURCE_DIR}/src/platinum_wmb4_tool.cpp
	#${PROJECT_SOURCE_DIR}/src/platinum_misc_dat2dds2dat.c
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(${PROJECT_SOURCE_DIR}/kwaslib)

foreach(program_src ${KWASTOOLS_PROGRAMS})
	get_filename_component(program_name ${program_src} NAME_WLE)
	if(WIN32)
        add_executable(${program_name} ${program_src})
    else()
        add_executable(${program_name} ${program_src})
    endif()
	target_include_directories(${program_name} PRIVATE "${KWASTOOLS_UTILS_INCLUDES}")
	target_link_libraries(${program_name} kwaslib)
endforeach(program_src ${KWASTOOLS_PROGRAMS})

# Create bin directory
file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")