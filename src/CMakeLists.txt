include_directories(
    ${PyClassifiers_SOURCE_DIR}/lib/Files
    ${PyClassifiers_SOURCE_DIR}/lib/json/include
    ${Python3_INCLUDE_DIRS}
    ${TORCH_INCLUDE_DIRS}
    /usr/local/include/bayesnet
)
add_library(PyClassifiers ${PyClassifiers_SOURCES})
target_link_libraries(PyClassifiers ${Python3_LIBRARIES} "${TORCH_LIBRARIES}" ${LIBTORCH_PYTHON} Boost::boost Boost::python Boost::numpy)