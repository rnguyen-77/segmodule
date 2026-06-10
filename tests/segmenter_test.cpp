#include <gtest/gtest.h>
#include "../segmenter.hpp"
#include "../segmenterFactory.hpp"

// ── ConfigLoader ─────────────────────────────────────────────────────────────

class ConfigLoaderTest : public ::testing::Test {
protected:
    std::string validPath   = "tests/test_config.yaml";
    std::string invalidPath = "nonexistent.yaml";
};

TEST_F(ConfigLoaderTest, ThrowsOnMissingFile) {
    EXPECT_THROW(ConfigLoader(invalidPath), std::exception);
}

TEST_F(ConfigLoaderTest, LoadsActiveStyle) {
    // hint: EXPECT_EQ
    ConfigLoader config(validPath);
    EXPECT_EQ(config.getActiveStyle(), "Canny");
}

TEST_F(ConfigLoaderTest, GetParamReturnsCorrectValue) {
    // hint: EXPECT_DOUBLE_EQ
    ConfigLoader config(validPath);
     std::vector<std::tuple<std::string, std::string, double>> cases = {
          {"Threshold", "threshold", 120.0},                                                                                                                                           
          {"Threshold", "maxValue", 255.0},                                                                                                                                           
          {"Canny", "lowThreshold", 50.0},
          {"Canny", "highThreshold", 150.0},                                                                                                                                           
      };          
                                                                                                                                                                                           
      for (const auto& [style, param, expected] : cases) {                                                                                                                                 
          EXPECT_DOUBLE_EQ(config.getParam(style, param), expected)
              << "Failed for style=" << style << " param=" << param;                                                                                                                       
      }           
} 

TEST_F(ConfigLoaderTest, GetParamThrowsOnUnknownStyle) {
    // hint: EXPECT_THROW with std::runtime_error
    ConfigLoader config(validPath);
    std::string badStyle = "testStyle";
    std::string badParam = "testParam";

    EXPECT_THROW(config.getParam(badStyle, "lowThreshold"), std::runtime_error);
    EXPECT_THROW(config.getParam(config.getActiveStyle(), badParam), std::runtime_error);
}

// ── Objects ───────────────────────────────────────────────────────────────────

class ObjectsTest : public ::testing::Test {
protected:
    Objects obj{1};
};

TEST_F(ObjectsTest, SetAndGetRoundTrip) {
    // hint: EXPECT_DOUBLE_EQ after std::get<double>
    obj.setFeature("area", 44.0);
    EXPECT_DOUBLE_EQ(obj.getFeature("area"), 44.0);
}

TEST_F(ObjectsTest, HasReturnsTrueAfterSet) {
    // hint: EXPECT_TRUE
    obj.setFeature("area", 44.0);
    EXPECT_TRUE(obj.hasFeature("area"));
}

TEST_F(ObjectsTest, HasReturnsFalseForMissingFeature) {
    // hint: EXPECT_FALSE
    EXPECT_FALSE(obj.hasFeature("area"));
}

TEST_F(ObjectsTest, GetThrowsOnMissingFeature) {
    // hint: EXPECT_THROW with std::out_of_range
    EXPECT_THROW(obj.getFeature("area"), std::out_of_range);
}

// ── ThresholdSegmenter ────────────────────────────────────────────────────────

class ThresholdSegmenterTest : public ::testing::Test {
protected:
    void SetUp() override {
        segmenter = std::make_unique<ThresholdSegmenter>(128.0, 255.0);
    }

    std::unique_ptr<ThresholdSegmenter> segmenter;
    ALOG alogImage;
    ALOG alogLabelImage;
    ObjectCollection objects;
};

TEST_F(ThresholdSegmenterTest, SegmentProducesOutputObjects) {
    // hint: EXPECT_FALSE on objects.empty()
    segmenter->segment(alogImage, alogLabelImage, objects);
    EXPECT_FALSE(objects.size() == 0);
}

TEST_F(ThresholdSegmenterTest, SegmentPopulatesExpectedFeatures) {
    // hint: EXPECT_TRUE on objects[0].has(...)
    segmenter->segment(alogImage, alogLabelImage, objects);
    ASSERT_FALSE(objects.size() == 0);
    EXPECT_TRUE(objects.all()[0].hasFeature("Area"));
}

// ── CannySegmenter ────────────────────────────────────────────────────────────

class CannySegmenterTest : public ::testing::Test {
protected:
    void SetUp() override {
        segmenter = std::make_unique<CannySegmenter>(50.0, 150.0);
    }

    std::unique_ptr<CannySegmenter> segmenter;
    ALOG alogImage;
    ALOG alogLabelImage;
    ObjectCollection objects;
};

TEST_F(CannySegmenterTest, SegmentProducesOutputObjects) {
    // hint: EXPECT_FALSE on objects.empty()
    segmenter->segment(alogImage, alogLabelImage, objects);
    EXPECT_FALSE(objects.size()==0);
}

TEST_F(CannySegmenterTest, SegmentPopulatesExpectedFeatures) {
    // hint: EXPECT_TRUE on objects[0].has(...)
    segmenter->segment(alogImage, alogLabelImage, objects);
    ASSERT_FALSE(objects.size()==0);
    EXPECT_TRUE(objects.all()[0].hasFeature("Area"));
}

// ── LabelImage ───────────────────────────────────────────────────────────────

class LabelImageTest : public ::testing::Test {
protected:
    void SetUp() override {
        //synthetic input: 100x100 black image with one filled white rectangle (passed to the segmenter)
        syntheticInput = cv::Mat::zeros(100, 100, CV_8UC1);
        cv::rectangle(syntheticInput, cv::Rect(10, 10, 30, 30), cv::Scalar(255), cv::FILLED);

        //expected label image: same geometry, rectangle filled with label value 1
        //connectedComponents assigns 0 to background and 1 to the single blob; output type is CV_32S
        expectedLabelMat = cv::Mat::zeros(100, 100, CV_32S);
        cv::rectangle(expectedLabelMat, cv::Rect(10, 10, 30, 30), cv::Scalar(1), cv::FILLED);

        //ALOG alogInput = ALOG(syntheticInput); // wrap synthetic input in ALOG when class is ready
        segmenter = std::make_unique<ThresholdSegmenter>(160, 255);
    }

    cv::Mat syntheticInput;         // synthetic grayscale input image (fed into the segmenter)
    cv::Mat expectedLabelMat;       // hand-built expected label image (compared against output)
    ALOG alogInput;
    ALOG alogLabelImage;
    ObjectCollection objects;
    std::unique_ptr<ThresholdSegmenter> segmenter;
};

TEST_F(LabelImageTest, LabelImageIsNotEmpty) {
    // hint: EXPECT_FALSE on labelImage being empty — check cv::Mat via convertALOGtoMat when ready
    segmenter->segment(alogInput, alogLabelImage, objects);
    EXPECT_FALSE(alogLabelImage.empty());
}

TEST_F(LabelImageTest, LabelImageMatchesExpected) {
    //compare the segmenter's label image against the hand-built expected label image
    alogInput = ALOG(syntheticInput); //placeholder
    segmenter->segment(alogInput, alogLabelImage, objects);
    cv::Mat actualLabelMat;  // = convertALOGtoMat(alogLabelImage);  ← wire up once ALOG exposes its cv::Mat
    EXPECT_EQ(cv::norm(expectedLabelMat, actualLabelMat, cv::NORM_INF), 0);
}

// ── SegmenterFactory ──────────────────────────────────────────────────────────

class SegmenterFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_unique<ConfigLoader>("tests/test_config.yaml");
    }

    std::unique_ptr<ConfigLoader> config;
};

TEST_F(SegmenterFactoryTest, CreatesThresholdSegmenter) {
    // hint: EXPECT_NE against nullptr
    auto factory = SegmenterFactory::factoryStyle("Threshold");
    auto seg = factory->create(*config);
    EXPECT_NE(seg, nullptr);
}

TEST_F(SegmenterFactoryTest, CreatesCannySegmenter) {
    // hint: EXPECT_NE against nullptr
    auto factory = SegmenterFactory::factoryStyle("Canny");
    auto seg = factory->create(*config);
    EXPECT_NE(seg, nullptr);
}

TEST_F(SegmenterFactoryTest, ThrowsOnUnknownStyle) {
    // hint: EXPECT_THROW with std::invalid_argument
    EXPECT_THROW(SegmenterFactory::factoryStyle("Unknown"), std::invalid_argument);
}

// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
