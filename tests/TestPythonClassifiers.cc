#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
#include <vector>
#include <map>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <nlohmann/json.hpp>
#include "pyclfs/STree.h"
#include "pyclfs/SVC.h"
#include "pyclfs/RandomForest.h"
#include "pyclfs/XGBoost.h"
#include "pyclfs/ODTE.h"
#include "TestUtils.h"

TEST_CASE("Test Python Classifiers score", "[PyClassifiers]")
{
    map <pair<std::string, std::string>, float> scores = {
        // Diabetes
        {{"diabetes", "STree"}, 0.81641}, {{"diabetes", "ODTE"}, 0.854166687}, {{"diabetes", "SVC"}, 0.76823}, {{"diabetes", "RandomForest"}, 1.0},
        // Ecoli
        {{"ecoli", "STree"}, 0.8125}, {{"ecoli", "ODTE"}, 0.875}, {{"ecoli", "SVC"}, 0.89583}, {{"ecoli", "RandomForest"}, 1.0},
        // Glass
        {{"glass", "STree"}, 0.57009}, {{"glass", "ODTE"}, 0.76168227}, {{"glass", "SVC"}, 0.35514}, {{"glass", "RandomForest"}, 1.0},
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
        {"ODTE", "1.0.0-1"},
        {"STree", "1.4.0"},
        {"SVC", "1.5.1"},
        {"RandomForest", "1.5.1"}
    };
    auto clf = models[name];

    SECTION("Test Python Classifier " + name + " score ")
    {
        auto random_state = nlohmann::json::parse("{ \"random_state\": 0 }");
        for (std::string file_name : { "glass", "iris", "ecoli", "diabetes" }) {
            auto raw = RawDatasets(file_name, false);
            clf->setHyperparameters(random_state);
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
    auto raw = RawDatasets("iris", false);
    auto clf = pywrap::STree();
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    REQUIRE(clf.getNumberOfNodes() == 5);
    REQUIRE(clf.getNumberOfEdges() == 3);
}
TEST_CASE("Get num features & num edges", "[PyClassifiers]")
{
    auto estimators = nlohmann::json::parse("{ \"n_estimators\": 10 }");
    auto raw = RawDatasets("iris", false);
    auto clf = pywrap::ODTE();
    clf.setHyperparameters(estimators);
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    REQUIRE(clf.getNumberOfNodes() == 50);
    REQUIRE(clf.getNumberOfEdges() == 30);
}
TEST_CASE("Classifier with discretized dataset", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", true);
    auto clf = pywrap::SVC();
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    auto score = clf.score(raw.Xt, raw.yt);
    REQUIRE(score == Catch::Approx(0.96667f).epsilon(raw.epsilon));
}
TEST_CASE("Predict with non_discretized dataset and comparing to predict_proba", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", false);
    auto clf = pywrap::STree();
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    auto predictions = clf.predict(raw.Xt);
    auto probabilities = clf.predict_proba(raw.Xt);
    auto preds = probabilities.argmax(1);
    auto classNumStates = torch::max(raw.yt).item<int>() + 1;

    REQUIRE(predictions.size(0) == probabilities.size(0));
    REQUIRE(predictions.size(0) == preds.size(0));
    REQUIRE(probabilities.size(1) == classNumStates);
    int right = 0;
    for (std::size_t i = 0; i < predictions.size(0); ++i) {
        if (predictions[i].item<int>() == preds[i].item<int>()) {
            right++;
        }
        REQUIRE(predictions[i].item<int>() == preds[i].item<int>());
    }
    auto accuracy = right / static_cast<float>(predictions.size(0));
    REQUIRE(accuracy == Catch::Approx(1.0f).epsilon(raw.epsilon));
}
TEST_CASE("XGBoost", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", true);
    auto clf = pywrap::XGBoost();
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    nlohmann::json hyperparameters = { "n_jobs=1" };
    clf.setHyperparameters(hyperparameters);
    auto score = clf.score(raw.Xt, raw.yt);
    REQUIRE(score == Catch::Approx(0.98).epsilon(raw.epsilon));
    std::cout << "XGBoost score: " << score << std::endl;
}
TEST_CASE("XGBoost predict proba", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", true);
    auto clf = pywrap::XGBoost();
    clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
    // nlohmann::json hyperparameters = { "n_jobs=1" };
    // clf.setHyperparameters(hyperparameters);
    auto predict_proba = clf.predict_proba(raw.Xt);
    auto predict = clf.predict(raw.Xt);
    // std::cout << "Predict proba: " << predict_proba << std::endl;
    // std::cout << "Predict proba size: " << predict_proba.sizes() << std::endl;
    // assert(predict.size(0) == predict_proba.size(0));
    for (int row = 0; row < predict_proba.size(0); row++) {
        // auto sum = 0.0;
        // std::cout << "Row " << std::setw(3) << row << ": ";
        // for (int col = 0; col < predict_proba.size(1); col++) {
        //     std::cout << std::setw(9) << std::fixed << std::setprecision(7) << predict_proba[row][col].item<double>() << " ";
        //     sum += predict_proba[row][col].item<double>();
        // }
        // std::cout << " -> " << std::setw(9) << std::fixed << std::setprecision(7) << sum << " -> " << torch::argmax(predict_proba[row]).item<int>() << " = " << predict[row].item<int>() << std::endl;
        //     // REQUIRE(sum == Catch::Approx(1.0).epsilon(raw.epsilon));
        REQUIRE(torch::argmax(predict_proba[row]).item<int>() == predict[row].item<int>());
        REQUIRE(torch::sum(predict_proba[row]).item<double>() == Catch::Approx(1.0).epsilon(raw.epsilon));
    }
}