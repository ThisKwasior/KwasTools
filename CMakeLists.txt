cmake_minimum_required(VERSION 3.19)
project(KwasTools)

set(KWASTOOLS_STATIC_UTILSLIB False CACHE STRING "Should KwasToolsUtils be a static library?")

set(KWASTOOLS_PROGRAMS
	${PROJECT_SOURCE_DIR}/src/pcmodeltool.cpp
	)

set(KWASTOOLS_UTILS
	${PROJECT_SOURCE_DIR}/utils/pugixml.cpp
	)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -Qn -fno-ident -fPIC" CACHE STRING "C Compiler flags" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} " CACHE STRING "CXX Compiler flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -fPIC" CACHE STRING "EXE link flags" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(${PROJECT_SOURCE_DIR}/kwaslib)

# utils library
if(${KWASTOOLS_STATIC_UTILSLIB})
	add_library(KwasToolsUtils STATIC ${KWASTOOLS_UTILS})
else()
	add_library(KwasToolsUtils SHARED ${KWASTOOLS_UTILS})
endif()

target_link_libraries(KwasToolsUtils)
# end utils library

foreach(program_src ${KWASTOOLS_PROGRAMS})
	get_filename_component(program_name ${program_src} NAME_WLE)
	add_executable(${program_name} ${program_src})
	target_include_directories(${program_name} PRIVATE "${PROJECT_SOURCE_DIR}")
	target_link_libraries(${program_name} KwasToolsUtils kwaslib)
endforeach(program_src ${KWASTOOLS_PROGRAMS})