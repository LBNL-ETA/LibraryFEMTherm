cmake_minimum_required(VERSION 3.8)

project(LibraryFEMTHERM VERSION 1.0.0)
set(target_name LibraryFEMTHERM)

include(cmake/LibraryFEMTHERMProjectMacros.cmake)
include(cmake/LibraryFEMTHERMinternal_utils.cmake)
include(cmake/LibraryFEMTHERMCompilerFlags.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(GLOBAL_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Sets global output directory for single configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})

# google test will not be created by this project if this is not the main project. Main project is expected to handle google test.
get_directory_property(hasParent PARENT_DIRECTORY)

if(hasParent)
    set(BUILD_LibraryFEMTHERM_TESTING OFF)
    set(DOWNLOAD_GTEST OFF)
    if(BUILD_TESTING STREQUAL ON) # Parent testing is ON
        set(BUILD_KeffC_TESTING ON)
    endif()
else()
    option(BUILD_LibraryFEMTHERM_TESTING "Build testing targets" ON)
    set(DOWNLOAD_GTEST ON)
endif()

# Download external projects automatically
option(BUILD_FileParse_tests "Build the FileParse tests" OFF)
include(CMakeLists-FileParse.txt)
include(CMakeLists-miniz.txt)

include_directories(src)
include_directories(${CMAKE_BINARY_DIR}/xmlParser-src/include)

# Download and include GoogleTest directly if needed
if(DOWNLOAD_GTEST)
    configure_file(CMakeLists-GTest.txt.in ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

	# Prevent GoogleTest from overriding our compiler/linker options
	# when building with Visual Studio
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest directly to our build. This adds
	# the following targets: gtest, gtest_main, gmock
	# and gmock_main
	add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
			${CMAKE_BINARY_DIR}/googletest-build)

	# Now simply link your own targets against gtest, gmock,
	# etc. as appropriate

	include_directories( ${CMAKE_BINARY_DIR}/googletest-src/include )
endif()

file(GLOB_RECURSE SOURCES_HXX "src/*.hxx")
file(GLOB_RECURSE SOURCES_CXX "src/*.cxx")
LIST(APPEND SOURCES ${SOURCES_HXX} ${SOURCES_CXX})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library(${target_name} STATIC ${SOURCES})

set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(${target_name} PUBLIC FileParse miniz)

if(BUILD_LibraryFEMTHERM_TESTING)
    add_subdirectory(tst)
endif()

config_compiler_and_linker_LibraryFEMTHERM()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /utf-8")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /utf-8")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /utf-8")
endif()