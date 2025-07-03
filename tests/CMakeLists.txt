if(ENABLE_TESTING)
    set(TEST_PYCLASSIFIERS "unit_tests_pyclassifiers")
    include_directories(
        ${PyClassifiers_SOURCE_DIR}
        ${Python3_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}/configured_files/include
    )
    file(GLOB_RECURSE PyClassifiers_SOURCES "${PyClassifiers_SOURCE_DIR}/pyclfs/*.cc")
    set(TEST_SOURCES_PYCLASSIFIERS TestPythonClassifiers.cc TestUtils.cc ${PyClassifiers_SOURCES})
    add_executable(${TEST_PYCLASSIFIERS} ${TEST_SOURCES_PYCLASSIFIERS})
    target_link_libraries(${TEST_PYCLASSIFIERS} PUBLIC 
      torch::torch ${Python3_LIBRARIES} ${LIBTORCH_PYTHON} 
      Boost::boost Boost::python Boost::numpy fimdlp::fimdlp
      Catch2::Catch2WithMain nlohmann_json::nlohmann_json 
      bayesnet::bayesnet
    )
endif(ENABLE_TESTING)
