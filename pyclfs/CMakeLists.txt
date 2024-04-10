include_directories(
    ${Python3_INCLUDE_DIRS}
    ${TORCH_INCLUDE_DIRS}
    ${PyClassifiers_SOURCE_DIR}/lib/json/include
    ${Bayesnet_INCLUDE_DIRS}
)
add_library(PyClassifiers ODTE.cc STree.cc SVC.cc RandomForest.cc XGBoost.cc PyClassifier.cc PyWrap.cc)
target_link_libraries(PyClassifiers ${Python3_LIBRARIES} "${TORCH_LIBRARIES}" ${LIBTORCH_PYTHON} Boost::boost Boost::python Boost::numpy)