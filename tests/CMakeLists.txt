if(ENABLE_TESTING)
    set(TEST_PYCLASSIFIERS "unit_tests_pyclassifiers")
    include_directories(
        ${PyClassifiers_SOURCE_DIR}/src/
        ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/lib/json/include
        ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/lib/Files
        ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/lib/mdlp
        ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src
        ${Python3_INCLUDE_DIRS}
        ${TORCH_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}/configured_files/include
    )
    set(TEST_SOURCES_PYCLASSIFIERS TestPythonClassifiers.cc TestUtils.cc ${PyClassifiers_SOURCES})
    add_executable(${TEST_PYCLASSIFIERS} ${TEST_SOURCES_PYCLASSIFIERS})
    target_link_libraries(${TEST_PYCLASSIFIERS} PUBLIC "${TORCH_LIBRARIES}" ${Python3_LIBRARIES} ${LIBTORCH_PYTHON} Boost::boost Boost::python Boost::numpy ArffFiles mdlp Catch2::Catch2WithMain)
    add_test(NAME ${TEST_PYCLASSIFIERS} COMMAND ${TEST_PYCLASSIFIERS})
endif(ENABLE_TESTING)