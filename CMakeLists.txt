cmake_minimum_required(VERSION 3.8)

# Force all targets to use the dynamic runtime library on MSVC
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(LibraryFEMTHERM VERSION 0.10.0 LANGUAGES CXX)
set(target_name LibraryFEMTHERM)

include(cmake/LibraryFEMTHERMProjectMacros.cmake)
include(cmake/LibraryFEMTHERMinternal_utils.cmake)
include(cmake/LibraryFEMTHERMCompilerFlags.cmake)

set(CMAKE_CXX_STANDARD 20)
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

include(FetchContent)

set(LBNLCPPCommon_Branch "v0.12")

FetchContent_Declare(
        LBNLCPPCommon
        GIT_REPOSITORY https://github.com/LBNL-ETA/LBNLCPPCommon.git
        GIT_TAG ${LBNLCPPCommon_Branch}
)

FetchContent_MakeAvailable(LBNLCPPCommon)

set(FileParse_Branch "Version_1.0.5")

FetchContent_Declare(
    FileParse
    GIT_REPOSITORY https://github.com/LBNL-ETA/FileParse.git
    GIT_TAG ${FileParse_Branch}
)

FetchContent_MakeAvailable(FileParse)

set(miniz_Branch "89d7a5f6c3ce8893ea042b0a9d2a2d9975589ac9")

FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz
    GIT_TAG ${miniz_Branch}
)
FetchContent_MakeAvailable(miniz)

if(DOWNLOAD_GTEST)
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    if(NOT TARGET gtest)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.13.0
        )
        FetchContent_MakeAvailable(googletest)

        include_directories(${gtest_SOURCE_DIR}/include)
    endif()
endif()

file(GLOB_RECURSE SOURCES_HXX "src/*.hxx")
file(GLOB_RECURSE SOURCES_CXX "src/*.cxx")
LIST(APPEND SOURCES ${SOURCES_HXX} ${SOURCES_CXX})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library(${target_name} STATIC ${SOURCES})

set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${target_name}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${fileparse_SOURCE_DIR}/include
        ${LBNLCPPCommon_SOURCE_DIR}/include
)

target_link_libraries(${target_name} PRIVATE FileParse LBNLCPPCommon miniz)

set_property(TARGET ${target_name} PROPERTY FileParse_Branch ${FileParse_Branch})
set_property(TARGET ${target_name} PROPERTY miniz_Branch ${miniz_Branch})

if(BUILD_LibraryFEMTHERM_TESTING)
    add_subdirectory(tst)
    # Add include directories for tests if necessary
    set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tst")
    target_compile_definitions(LibraryFEMTHERM_tests PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
endif()

config_compiler_and_linker_LibraryFEMTHERM()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /utf-8")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /utf-8")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /utf-8")
endif()

# Warning levels
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()