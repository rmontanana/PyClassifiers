if(ENABLE_TESTING)
    set(TEST_PYCLASSIFIERS "unit_tests_pyclassifiers")
    include_directories(
        ${PyClassifiers_SOURCE_DIR}
        ${PyClassifiers_SOURCE_DIR}/lib/Files
        ${PyClassifiers_SOURCE_DIR}/lib/mdlp
        ${PyClassifiers_SOURCE_DIR}/lib/json/include
        ${Python3_INCLUDE_DIRS}
        ${TORCH_INCLUDE_DIRS}
        /usr/local/include
    )
    file(GLOB_RECURSE PyClassifiers_SOURCES "${PyClassifiers_SOURCE_DIR}/pyclfs/*.cc")
    set(TEST_SOURCES_PYCLASSIFIERS TestPythonClassifiers.cc TestUtils.cc ${PyClassifiers_SOURCES})
    add_executable(${TEST_PYCLASSIFIERS} ${TEST_SOURCES_PYCLASSIFIERS})
    target_link_libraries(${TEST_PYCLASSIFIERS} PUBLIC "${TORCH_LIBRARIES}" ${Python3_LIBRARIES} ${LIBTORCH_PYTHON} Boost::boost Boost::python Boost::numpy ArffFiles mdlp Catch2::Catch2WithMain)
endif(ENABLE_TESTING)