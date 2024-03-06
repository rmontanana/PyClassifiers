include_directories(
    ${PyClassifiers_SOURCE_DIR}/lib/Files
    ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/lib/json/include
    ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src
    ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src/classifiers
    ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src/ensembles
    ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src/bayesian_network
    ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src/utils
    ${CMAKE_BINARY_DIR}/configured_files/include
    ${Python3_INCLUDE_DIRS}
    ${TORCH_INCLUDE_DIRS}
)
add_library(PyClassifiers SHARED ${PyClassifiers_SOURCES})
target_link_libraries(PyClassifiers BayesNet ${Python3_LIBRARIES} "${TORCH_LIBRARIES}" ${LIBTORCH_PYTHON} Boost::boost Boost::python Boost::numpy)