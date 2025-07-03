include_directories(
    ${Python3_INCLUDE_DIRS}
    ${PyClassifiers_SOURCE_DIR}/lib/json/include
)
add_library(PyClassifiers ODTE.cc STree.cc SVC.cc RandomForest.cc XGBoost.cc AdaBoostPy.cc PyClassifier.cc PyWrap.cc)
target_link_libraries(PyClassifiers PRIVATE 
  nlohmann_json::nlohmann_json torch::torch 
  Boost::boost Boost::python Boost::numpy 
  bayesnet::bayesnet
)
