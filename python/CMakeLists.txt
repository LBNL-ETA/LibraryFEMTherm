file(GLOB_RECURSE BINDING_SOURCES "src/*.cxx")

pybind11_add_module(pylibraryfemtherm ${BINDING_SOURCES})

target_link_libraries(pylibraryfemtherm PRIVATE LibraryFEMTHERM)

target_include_directories(pylibraryfemtherm PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${fileparse_SOURCE_DIR}/include
    ${LBNLCPPCommon_SOURCE_DIR}/include)

set_target_properties(pylibraryfemtherm PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS pylibraryfemtherm LIBRARY DESTINATION .)

# Register pytest as a CTest test so CLion / ctest runs both C++ and Python tests
add_test(NAME python_bindings
    COMMAND ${Python3_EXECUTABLE} -m pytest "${CMAKE_CURRENT_SOURCE_DIR}/tests" -v
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_tests_properties(python_bindings PROPERTIES
    ENVIRONMENT "FEMTHERM_MODULE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
