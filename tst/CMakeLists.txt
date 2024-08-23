if(BUILD_LibraryFEMTHERM_TESTING)
    include_directories(${CMAKE_SOURCE_DIR}/src)
    include_directories(${CMAKE_SOURCE_DIR}/tst/helper)

    file(GLOB all_test_src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
            "main.cxx"
            "units/Boundary Conditions/*.cxx"
            "units/CMA/*.cxx"
            "units/Common/*.cxx"
            "units/Gases/*.cxx"
            "units/Geometry/*.cxx"
            "units/Materials/*.cxx"
            "units/THMZ/CMA/*.cxx"
            "units/THMZ/Common/*.cxx"
            "units/THMZ/Geometry/*.cxx"
            "units/THMZ/GlazingSystem/*.cxx"
            "units/THMZ/Mesh/*.cxx"
            "units/THMZ/Preferences/*.cxx"
            "units/THMZ/Properties/*.cxx"
            "units/THMZ/SteadyStateMeshResults/*.cxx"
            "units/THMZ/SteadyStateResults/*.cxx"
            "units/THMZ/*.cxx"
            "helper/Common/*.hxx"
            "helper/*.hxx"
            "helper/*.cxx")

    list(APPEND test_src ${all_test_src})

    CREATE_TEST_TARGETS_LibraryFEMTHERM(${target_name} "${test_src}" "")
endif()
