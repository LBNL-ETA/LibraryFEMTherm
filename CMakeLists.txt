cmake_minimum_required(VERSION 3.8)

project(LibraryFEMTHERM VERSION 1.0.0)
set(target_name LibraryFEMTHERM)

include(cmake/LibraryFEMTHERMProjectMacros.cmake)
include(cmake/LibraryFEMTHERMinternal_utils.cmake)
include(cmake/LibraryFEMTHERMCompilerFlags.cmake)

set(CMAKE_CXX_STANDARD 17)

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
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.13.0
    )

    FetchContent_MakeAvailable(googletest)

    if(MSVC)
        # Force all targets, including external ones, to use the same runtime library
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    # Link GTest with test targets
    include_directories(${gtest_SOURCE_DIR}/include)
    include_directories(${gmock_SOURCE_DIR}/include)
endif()

file(GLOB_RECURSE SOURCES_HXX "src/*.hxx")
file(GLOB_RECURSE SOURCES_CXX "src/*.cxx")
LIST(APPEND SOURCES ${SOURCES_HXX} ${SOURCES_CXX})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library(${target_name} STATIC ${SOURCES})

set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)

# Configure runtime library for different build types
if(MSVC)
    # Set the correct runtime library flags for the entire project
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MD")
endif()

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