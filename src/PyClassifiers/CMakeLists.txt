include_directories(
    ${PyClassifiers_SOURCE_DIR}/lib/Files
    ${PyClassifiers_SOURCE_DIR}/lib/json/include
    ${PyClassifiers_SOURCE_DIR}/lib/BayesNet/src/BayesNet
    ${CMAKE_BINARY_DIR}/configured_files/include
    ${Python3_INCLUDE_DIRS}
    ${TORCH_INCLUDE_DIRS}
)
add_library(PyClassifiers SHARED PyWrap.cc STree.cc ODTE.cc SVC.cc RandomForest.cc PyClassifier.cc)
#target_link_libraries(PyClassifiers ${Python3_LIBRARIES} "${TORCH_LIBRARIES}" ${LIBTORCH_PYTHON} Boost::boost Boost::python Boost::numpy xgboost::xgboost)
target_link_libraries(PyClassifiers ${Python3_LIBRARIES} "${TORCH_LIBRARIES}" ${LIBTORCH_PYTHON} Boost::boost Boost::python Boost::numpy)