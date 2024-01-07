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
#include "ODTE.h"
#include "TestUtils.h"

TEST_CASE("Test Python Classifiers score", "[PyClassifiers]")
{
    map <pair<std::string, std::string>, float> scores = {
        // Diabetes
        {{"diabetes", "STree"}, 0}, {{"diabetes", "ODTE"}, 0.84635}, {{"diabetes", "SVC"}, 0}, {{"diabetes", "RandomForest"}, 1.0},
        // Ecoli
        {{"ecoli", "STree"}, 0}, {{"ecoli", "ODTE"}, 0.84821}, {{"ecoli", "SVC"}, 0.}, {{"ecoli", "RandomForest"}, 1.0},
        // Glass
        {{"glass", "STree"}, 0}, {{"glass", "ODTE"}, 0.77103}, {{"glass", "SVC"}, 0}, {{"glass", "RandomForest"}, 1.0},
        // Iris
        {{"iris", "STree"}, 0}, {{"iris", "ODTE"}, 0.98667}, {{"iris", "SVC"}, 0}, {{"iris", "RandomForest"}, 1.0},
    };

    std::string file_name = GENERATE("glass", "iris", "ecoli", "diabetes");
    auto raw = RawDatasets(file_name, false);

    SECTION("Test STree classifier (" + file_name + ")")
    {
        auto clf = pywrap::STree();
        clf.fit(raw.Xv, raw.yv, raw.featuresv, raw.classNamev, raw.statesv);
        auto score = clf.score(raw.Xv, raw.yv);
        REQUIRE(score == Catch::Approx(scores[{file_name, "STree"}]).epsilon(raw.epsilon));
    }
    SECTION("Test ODTE classifier (" + file_name + ")")
    {
        auto clf = pywrap::ODTE();
        clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
        auto score = clf.score(raw.Xt, raw.yt);
        scores[{file_name, "ODTE"}] = score;
        REQUIRE(score == Catch::Approx(scores[{file_name, "ODTE"}]).epsilon(raw.epsilon));
    }
    SECTION("Test SVC classifier (" + file_name + ")")
    {
        auto clf = pywrap::SVC();
        clf.fit(raw.Xv, raw.yv, raw.featuresv, raw.classNamev, raw.statesv);
        auto score = clf.score(raw.Xv, raw.yv);
        scores[{file_name, "SVC"}] = score;
        REQUIRE(score == Catch::Approx(scores[{file_name, "SVC"}]).epsilon(raw.epsilon));
    }
    SECTION("Test RandomForest classifier (" + file_name + ")")
    {
        auto clf = pywrap::RandomForest();
        clf.fit(raw.Xt, raw.yt, raw.featurest, raw.classNamet, raw.statest);
        auto score = clf.score(raw.Xt, raw.yt);
        scores[{file_name, "RandomForest"}] = score;
        REQUIRE(score == Catch::Approx(scores[{file_name, "RandomForest"}]).epsilon(raw.epsilon));
    }
    for (auto scores : scores) {
        std::cout << "{{\"" << scores.first.first << "\", \"" << scores.first.second << "\"}, " << scores.second << "}, ";
    }
}
TEST_CASE("Classifiers features", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", true);
    auto clf = pywrap::STree();
    clf.fit(raw.Xv, raw.yv, raw.featuresv, raw.classNamev, raw.statesv);
    REQUIRE(clf.getNumberOfNodes() == 0);
    REQUIRE(clf.getNumberOfEdges() == 0);
}
TEST_CASE("Get num features & num edges", "[PyClassifiers]")
{
    auto raw = RawDatasets("iris", true);
    auto clf = pywrap::ODTE();
    clf.fit(raw.Xv, raw.yv, raw.featuresv, raw.classNamev, raw.statesv);
    REQUIRE(clf.getNumberOfNodes() == 5);
    REQUIRE(clf.getNumberOfEdges() == 8);
}