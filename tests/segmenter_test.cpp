#include <gtest/gtest.h>
#include "../segmenter.hpp"
#include "../segmenterFactory.hpp"

// ── ConfigLoader ─────────────────────────────────────────────────────────────

class ConfigLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // TODO: provide a minimal valid YAML test fixture file
        // validPath = "path/to/test_config.yaml";
    }

    void TearDown() override {}

    std::string validPath;
    std::string invalidPath = "nonexistent.yaml";
};

TEST_F(ConfigLoaderTest, ThrowsOnMissingFile) {
    EXPECT_THROW(ConfigLoader(invalidPath), std::exception);
}

TEST_F(ConfigLoaderTest, LoadsActiveStyle) {
    // ConfigLoader config(validPath);
    // EXPECT_EQ(config.getActiveStyle(), "expectedStyle");
}

TEST_F(ConfigLoaderTest, GetParamReturnsCorrectValue) {
    // ConfigLoader config(validPath);
    // EXPECT_DOUBLE_EQ(config.getParam("Threshold", "threshold"), expectedValue);
}

TEST_F(ConfigLoaderTest, GetParamThrowsOnUnknownStyle) {
    // ConfigLoader config(validPath);
    // EXPECT_THROW(config.getParam("NoSuchStyle", "threshold"), std::runtime_error);
}

TEST_F(ConfigLoaderTest, GetFeatureNamesReturnsExpectedNames) {
    // ConfigLoader config(validPath);
    // auto names = config.getFeatureNames();
    // EXPECT_EQ(names.size(), expectedCount);
}

// ── ObjectFeatures ────────────────────────────────────────────────────────────

class ObjectFeaturesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    ObjectFeatures obj;
};

TEST_F(ObjectFeaturesTest, SetAndGetRoundTrip) {
    // obj.set("area", 42.0);
    // EXPECT_DOUBLE_EQ(std::get<double>(obj.get("area")), 42.0);
}

TEST_F(ObjectFeaturesTest, HasReturnsTrueAfterSet) {
    // obj.set("area", 42.0);
    // EXPECT_TRUE(obj.has("area"));
}

TEST_F(ObjectFeaturesTest, HasReturnsFalseForMissingFeature) {
    // EXPECT_FALSE(obj.has("nonexistent"));
}

TEST_F(ObjectFeaturesTest, GetThrowsOnMissingFeature) {
    // EXPECT_THROW(obj.get("nonexistent"), std::out_of_range);
}

// ── ThresholdSegmenter ────────────────────────────────────────────────────────

class ThresholdSegmenterTest : public ::testing::Test {
protected:
    void SetUp() override {
        featureNames = {"area", "centroid"};
        segmenter = std::make_unique<ThresholdSegmenter>(128.0, 255.0, featureNames);
    }

    void TearDown() override {}

    std::vector<std::string> featureNames;
    std::unique_ptr<ThresholdSegmenter> segmenter;
};

TEST_F(ThresholdSegmenterTest, SegmentProducesOutputObjects) {
    // ALOG input = ...; ALOG labelImage; std::vector<ObjectFeatures> objects;
    // segmenter->segment(input, labelImage, objects);
    // EXPECT_FALSE(objects.empty());
}

TEST_F(ThresholdSegmenterTest, SegmentPopulatesExpectedFeatures) {
    // ALOG input = ...; ALOG labelImage; std::vector<ObjectFeatures> objects;
    // segmenter->segment(input, labelImage, objects);
    // EXPECT_TRUE(objects[0].has("area"));
}

// ── CannySegmenter ────────────────────────────────────────────────────────────

class CannySegmenterTest : public ::testing::Test {
protected:
    void SetUp() override {
        featureNames = {"area", "centroid"};
        segmenter = std::make_unique<CannySegmenter>(50.0, 150.0, featureNames);
    }

    void TearDown() override {}

    std::vector<std::string> featureNames;
    std::unique_ptr<CannySegmenter> segmenter;
};

TEST_F(CannySegmenterTest, SegmentProducesOutputObjects) {
    // ALOG input = ...; ALOG labelImage; std::vector<ObjectFeatures> objects;
    // segmenter->segment(input, labelImage, objects);
    // EXPECT_FALSE(objects.empty());
}

TEST_F(CannySegmenterTest, SegmentPopulatesExpectedFeatures) {
    // ALOG input = ...; ALOG labelImage; std::vector<ObjectFeatures> objects;
    // segmenter->segment(input, labelImage, objects);
    // EXPECT_TRUE(objects[0].has("area"));
}

// ── SegmenterFactory ──────────────────────────────────────────────────────────

class SegmenterFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // TODO: provide a valid config
        // config = std::make_unique<ConfigLoader>(validPath);
    }

    void TearDown() override {}

    // std::unique_ptr<ConfigLoader> config;
};

TEST_F(SegmenterFactoryTest, CreatesThresholdSegmenter) {
    // auto seg = SegmenterFactory::create("Threshold", *config);
    // EXPECT_NE(seg, nullptr);
}

TEST_F(SegmenterFactoryTest, CreatesCannySegmenter) {
    // auto seg = SegmenterFactory::create("Canny", *config);
    // EXPECT_NE(seg, nullptr);
}

TEST_F(SegmenterFactoryTest, ThrowsOnUnknownStyle) {
    // EXPECT_THROW(SegmenterFactory::create("NoSuchStyle", *config), std::invalid_argument);
}

// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
