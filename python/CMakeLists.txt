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

# Type stubs: after every build, pybind11-stubgen reads the module's signatures and writes
# them as a stub-only package, pylibraryfemtherm-stubs, next to the module. Editors and type
# checkers read it for types and completion; Python never loads it, so nothing changes at
# run time. Required, so the stubs always match the module they describe. --exit-code turns
# a signature that still names a C++ type into a build error instead of an untyped stub.
execute_process(COMMAND ${Python3_EXECUTABLE} -c "import pybind11_stubgen"
                RESULT_VARIABLE STUBGEN_MISSING
                OUTPUT_QUIET ERROR_QUIET)
if(STUBGEN_MISSING)
    message(FATAL_ERROR "The Python bindings need pybind11-stubgen to generate their type stubs:\n"
                        "  \"${Python3_EXECUTABLE}\" -m pip install pybind11-stubgen")
endif()

set(STUBS_DIR "$<TARGET_FILE_DIR:pylibraryfemtherm>/pylibraryfemtherm-stubs")
add_custom_command(TARGET pylibraryfemtherm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${STUBS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:pylibraryfemtherm>"
            ${Python3_EXECUTABLE} -m pybind11_stubgen pylibraryfemtherm
            --output-dir "$<TARGET_FILE_DIR:pylibraryfemtherm>"
            --root-suffix=-stubs
            --exit-code
            --enum-class-locations "FileFormat:pylibraryfemtherm"
            --enum-class-locations "RunType:pylibraryfemtherm"
            --enum-class-locations "CalculationMode:pylibraryfemtherm"
            --enum-class-locations "BCConvectionModel:pylibraryfemtherm"
    COMMENT "Generating type stubs for pylibraryfemtherm"
    VERBATIM)

install(DIRECTORY "${STUBS_DIR}" DESTINATION .)

# Register pytest as a CTest test so CLion / ctest runs both C++ and Python tests
add_test(NAME python_bindings
    COMMAND ${Python3_EXECUTABLE} -m pytest "${CMAKE_CURRENT_SOURCE_DIR}/tests" -v
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_tests_properties(python_bindings PROPERTIES
    ENVIRONMENT "FEMTHERM_MODULE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
