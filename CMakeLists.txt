cmake_minimum_required(VERSION 3.21)
project(KwasTools)

set(KWASTOOLS_STATIC_UTILSLIB False CACHE STRING "Should KwasToolsUtils be a static library?")

set(KWASTOOLS_PROGRAMS
	#${PROJECT_SOURCE_DIR}/src/pcmodeltool.cpp
	${PROJECT_SOURCE_DIR}/src/platinum_dat_tool.c
	${PROJECT_SOURCE_DIR}/src/platinum_wta_wtp_tool.c
	${PROJECT_SOURCE_DIR}/src/platinum_wtb_tool.c
	#${PROJECT_SOURCE_DIR}/src/platinum_wmb4_to_dae.cpp
	)

set(KWASTOOLS_UTILS
	${PROJECT_SOURCE_DIR}/utils/pugixml/pugixml.cpp
	)

set(KWASTOOLS_UTILS_INCLUDES
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/utils"
	#"${PROJECT_SOURCE_DIR}/utils/assimp/include"
	)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "C Compiler flags" FORCE)
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} " CACHE STRING "CXX Compiler flags" FORCE)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIC" CACHE STRING "EXE link flags" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(${PROJECT_SOURCE_DIR}/kwaslib)

# utils library
if(${KWASTOOLS_STATIC_UTILSLIB})
	add_library(KwasToolsUtils STATIC ${KWASTOOLS_UTILS})
else()
	add_library(KwasToolsUtils SHARED ${KWASTOOLS_UTILS})
endif()

# assimp
set(ASSIMP_BUILD_TESTS False)
set(ASSIMP_INSTALL False)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "All Assimp Importers" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "All Assimp Exporters" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Assimp Tests" FORCE)

set(ASSIMP_BUILD_COLLADA_IMPORTER ON)
set(ASSIMP_BUILD_COLLADA_EXPORTER ON)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
set(ASSIMP_BUILD_GLTF_EXPORTER ON)
#add_subdirectory(${PROJECT_SOURCE_DIR}/utils/assimp)

#target_link_libraries(KwasToolsUtils assimp kwaslib)
target_link_libraries(KwasToolsUtils kwaslib)
# end utils library

foreach(program_src ${KWASTOOLS_PROGRAMS})
	get_filename_component(program_name ${program_src} NAME_WLE)
	add_executable(${program_name} ${program_src})
	target_include_directories(${program_name} PRIVATE "${KWASTOOLS_UTILS_INCLUDES}")
	target_link_libraries(${program_name} KwasToolsUtils)
endforeach(program_src ${KWASTOOLS_PROGRAMS})

### Copying license files

# Create bin directory
file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# pugixml license
file(COPY_FILE "${PROJECT_SOURCE_DIR}/utils/pugixml/LICENSE.md" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/LICENSE-pugixml.md")

# assimp license
#file(COPY_FILE "${PROJECT_SOURCE_DIR}/utils/assimp/LICENSE" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/LICENSE-assimp.md")