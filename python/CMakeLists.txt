file(GLOB_RECURSE BINDING_SOURCES "src/*.cxx")

pybind11_add_module(_femtherm ${BINDING_SOURCES})

target_link_libraries(_femtherm PRIVATE LibraryFEMTHERM)

target_include_directories(_femtherm PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${fileparse_SOURCE_DIR}/include
    ${LBNLCPPCommon_SOURCE_DIR}/include)

set_target_properties(_femtherm PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
