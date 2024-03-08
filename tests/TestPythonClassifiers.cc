#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <vector>
#include <map>
#include <string>
#include "STree.h"
#include "SVC.h"
#include "RandomForest.h"
#include "XGBoost.h"
#include "ODTE.h"
#include "TestUtils.h"
#include <nlohmann/json.hpp>

const std::string ACTUAL_VERSION = "1.0.4";

TEST_CASE("Test Python Classifiers score", "[PyClassifiers]")
{
    map <pair<std::string, std::string>, float> scores = {
        // Diabetes
        {{"diabetes", "STree"}, 0.81641}, {{"diabetes", "ODTE"}, 0.84635}, {{"diabetes", "SVC"}, 0.76823}, {{"diabetes", "RandomForest"}, 1.0},
        // Ecoli
        {{"ecoli", "STree"}, 0.8125}, {{"ecoli", "ODTE"}, 0.84821}, {{"ecoli", "SVC"}, 0.89583}, {{"ecoli", "RandomForest"}, 1.0},
        // Glass
        {{"glass", "STree"}, 0.57009}, {{"glass", "ODTE"}, 0.77103}, {{"glass", "SVC"}, 0.35514}, {{"glass", "RandomForest"}, 1.0},
        // Iris
        {{"iris", "STree"}, 0.99333}, {{"iris", "ODTE"}, 0.98667}, {{"iris", "SVC"}, 0.97333}, {{"iris", "RandomForest"}, 1.0},
    };
    std::string name = GENERATE("ODTE", "STree", "SVC", "RandomForest");
    map<std::string, pywrap::PyClassifier*> models = {
        {"ODTE", new pywrap::ODTE()},
        {"STree", new pywrap::STree()},
        {"SVC", new pywrap::SVC()},
        {"RandomForest", new pywrap::RandomForest()}
    };
    map<std::string, std::string> versions = {
        {"ODTE", "0.3.6"},
        {"STree", "1.3.2"},
        {"SVC", "1.3.2"},
        {"RandomForest", "1.3.2"}
    };
    auto clf = models[name];

    SECTION("Test Python Classifier " + name + " score ")
    {
        for (std::string file_name : { "glass", "iris", "ecoli", "diabetes" }) {
            auto raw = RawDatasets(file_name, false);
            clf->fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
            auto score = clf->score(raw.Xt, raw.yt);
            INFO("File: " + file_name + " Classifier: " + name + " Score: " + to_string(score));
            REQUIRE(score == Catch::Approx(scores[{file_name, name}]).epsilon(raw.epsilon));
        }
    }
    SECTION("Library check version")
    {
        INFO("Checking version of " + name + " classifier");
        REQUIRE(clf->getVersion() == versions[name]);
    }
}
TEST_CASE("Classifiers features", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", true);
    auto clf = pywrap::STree();
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    REQUIRE(clf.getNumberOfNodes() == 3);
    REQUIRE(clf.getNumberOfEdges() == 2);
}
TEST_CASE("Get num features & num edges", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", true);
    auto clf = pywrap::ODTE();
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    REQUIRE(clf.getNumberOfNodes() == 10);
    REQUIRE(clf.getNumberOfEdges() == 10);
}
// TEST_CASE("XGBoost", "[PyClassifiers]")
// {
//     auto raw = RawDatasets("iris", true);
//     auto clf = pywrap::XGBoost();
//     clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
//     nlohmann::json hyperparameters = { "n_jobs=1" };
//     clf.setHyperparameters(hyperparameters);
//     auto score = clf.score(raw.Xt, raw.yt);
//     REQUIRE(score == Catch::Approx(0.98).epsilon(raw.epsilon));
// }