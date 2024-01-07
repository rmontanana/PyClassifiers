if(ENABLE_TESTING)
    set(TEST_PYCLASSIFIERS "unit_tests_pyclassifiers")
    include_directories(
        ${PyClassifiers_SOURCE_DIR}/src/PyClassifiers
        ${PyClassifiers_SOURCE_DIR}/src/Platform
        ${PyClassifiers_SOURCE_DIR}/lib/Files
        ${PyClassifiers_SOURCE_DIR}/lib/mdlp
        ${PyClassifiers_SOURCE_DIR}/lib/folding
        ${PyClassifiers_SOURCE_DIR}/lib/json/include
        ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src/BayesNet
        ${Python3_INCLUDE_DIRS}
        ${TORCH_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}/configured_files/include
    )
    set(TEST_SOURCES_PYCLASSIFIERS TestPythonClassifiers.cc TestUtils.cc ${PyClassifiers_SOURCES})
    add_executable(${TEST_PYCLASSIFIERS} ${TEST_SOURCES_PYCLASSIFIERS})
    target_link_libraries(${TEST_PYCLASSIFIERS} PUBLIC "${TORCH_LIBRARIES}" PyClassifiers ArffFiles mdlp Catch2::Catch2WithMain)
    add_test(NAME ${TEST_PYCLASSIFIERS} COMMAND ${TEST_PYCLASSIFIERS})
endif(ENABLE_TESTING)