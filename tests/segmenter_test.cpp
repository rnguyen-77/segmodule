#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include "../segmenter.hpp"
#include "../segmenterFactory.hpp"

// Resolves the test config fixture relative to the test executable itself, not the
// current working directory. CMake stages test_config.yaml next to the binary on
// each build (see the POST_BUILD step in CMakeLists.txt), so the tests pass no
// matter which directory segmenter_test is launched from. /proc/self/exe is the
// kernel's symlink to the running binary on Linux.
static std::string configPath() {
    namespace fs = std::filesystem;
    const fs::path exeDir = fs::read_symlink("/proc/self/exe").parent_path();
    return (exeDir / "test_config.yaml").string();
}

// ── ConfigLoader ─────────────────────────────────────────────────────────────

class ConfigLoaderTest : public ::testing::Test {
protected:
    std::string validPath   = configPath();
    std::string invalidPath = "nonexistent.yaml";
};

TEST_F(ConfigLoaderTest, ThrowsOnMissingFile) {
    EXPECT_THROW({ ConfigLoader cfg(invalidPath); }, std::exception);
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

// ── Object ───────────────────────────────────────────────────────────────────

class ObjectTest : public ::testing::Test {
protected:
    Object obj{1};
};

TEST_F(ObjectTest, SetAndGetRoundTrip) {
    // hint: EXPECT_DOUBLE_EQ after std::get<double>
    obj.setFeature("area", 44.0);
    EXPECT_DOUBLE_EQ(obj.getFeature("area"), 44.0);
}

TEST_F(ObjectTest, HasReturnsTrueAfterSet) {
    // hint: EXPECT_TRUE
    obj.setFeature("area", 44.0);
    EXPECT_TRUE(obj.hasFeature("area"));
}

TEST_F(ObjectTest, HasReturnsFalseForMissingFeature) {
    // hint: EXPECT_FALSE
    EXPECT_FALSE(obj.hasFeature("area"));
}

TEST_F(ObjectTest, GetThrowsOnMissingFeature) {
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
    SegImage alogImage;
    SegImage alogLabelImage;
    Objects objects;
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
    SegImage alogImage;
    SegImage alogLabelImage;
    Objects objects;
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

        alogInput = SegImage(syntheticInput); // wrap synthetic input in SegImage for the segmenter
        segmenter = std::make_unique<ThresholdSegmenter>(160, 255);
    }

    cv::Mat syntheticInput;         // synthetic grayscale input image (fed into the segmenter)
    cv::Mat expectedLabelMat;       // hand-built expected label image (compared against output)
    SegImage alogInput;
    SegImage alogLabelImage;
    Objects objects;
    std::unique_ptr<ThresholdSegmenter> segmenter;
};

TEST_F(LabelImageTest, LabelImageIsNotEmpty) {
    // hint: EXPECT_FALSE on labelImage being empty — check cv::Mat via convertSegImagetoMat when ready
    segmenter->segment(alogInput, alogLabelImage, objects);
    EXPECT_FALSE(alogLabelImage.empty());
}

TEST_F(LabelImageTest, LabelImageMatchesExpected) {
    //compare the segmenter's label image against the hand-built expected label image
    segmenter->segment(alogInput, alogLabelImage, objects);
    cv::Mat actualLabelMat = alogLabelImage.mat();  // convert the alog output to OpenCV mat for comparison 
    EXPECT_EQ(cv::norm(expectedLabelMat, actualLabelMat, cv::NORM_INF), 0);
}

// ── SegmenterFactory ──────────────────────────────────────────────────────────

class SegmenterFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_unique<ConfigLoader>(configPath());
    }

    std::unique_ptr<ConfigLoader> config;
};

TEST_F(SegmenterFactoryTest, CreatesThresholdSegmenter) {
    // hint: EXPECT_NE against nullptr
    auto factory = SegmenterFactoryBase::factoryStyle("Threshold");
    auto seg = factory->create(*config);
    EXPECT_NE(seg, nullptr);
}

TEST_F(SegmenterFactoryTest, CreatesCannySegmenter) {
    // hint: EXPECT_NE against nullptr
    auto factory = SegmenterFactoryBase::factoryStyle("Canny");
    auto seg = factory->create(*config);
    EXPECT_NE(seg, nullptr);
}

TEST_F(SegmenterFactoryTest, ThrowsOnUnknownStyle) {
    // hint: EXPECT_THROW with std::invalid_argument
    EXPECT_THROW(SegmenterFactoryBase::factoryStyle("Unknown"), std::invalid_argument);
}

// ── Custom event listener: per-test pass/fail visibility ─────────────────────
// Hooks GoogleTest's per-test events so the output shows each individual test's
// result (and, on failure, the assertion location + message) instead of only a
// final aggregate count. Same mechanism as the listener in the addition apptest.
namespace {
    const std::string GREEN = "\033[32m";
    const std::string RED   = "\033[31m";
    const std::string CYAN  = "\033[36m";
    const std::string DIM   = "\033[2m";
    const std::string RESET = "\033[0m";
}

class SegmenterTestListener : public ::testing::EmptyTestEventListener {
public:
    // Called right before a test body runs
    void OnTestStart(const ::testing::TestInfo& info) override {
        std::cout << CYAN << "▶ RUN   " << RESET
                  << info.test_suite_name() << "." << info.name() << std::endl;
    }

    // Called after a test body finishes — print PASS/FAIL for that specific test
    void OnTestEnd(const ::testing::TestInfo& info) override {
        const ::testing::TestResult* result = info.result();
        const std::string fullName =
            std::string(info.test_suite_name()) + "." + info.name();

        if (result->Passed()) {
            std::cout << GREEN << "  PASS  " << RESET << fullName
                      << DIM << " (" << result->elapsed_time() << " ms)" << RESET << "\n";
            ++passedCount_;
        } else {
            std::cout << RED << "  FAIL  " << RESET << fullName
                      << DIM << " (" << result->elapsed_time() << " ms)" << RESET << "\n";
            // show each failed assertion's file:line and message
            for (int i = 0; i < result->total_part_count(); ++i) {
                const ::testing::TestPartResult& part = result->GetTestPartResult(i);
                if (part.failed()) {
                    // file_name() is null for failures from uncaught exceptions
                    // (no source location); guard it before streaming.
                    const char* file = part.file_name();
                    const char* msg  = part.summary();
                    std::cout << RED << "        @ "
                              << (file ? file : "<uncaught exception>");
                    if (file) std::cout << ":" << part.line_number();
                    std::cout << RESET << "\n"
                              << DIM << "        " << (msg ? msg : "") << RESET << "\n";
                }
            }
            failedNames_.push_back(fullName);
        }
        std::cout << std::flush;
    }

    // Called once after the whole suite finishes — clean per-test summary
    void OnTestProgramEnd(const ::testing::UnitTest& /*unit*/) override {
        std::cout << "\n" << CYAN << "════════ Summary ════════" << RESET << "\n"
                  << GREEN << passedCount_ << " passed" << RESET << ", "
                  << (failedNames_.empty() ? GREEN : RED) << failedNames_.size()
                  << " failed" << RESET << "\n";
        for (const std::string& name : failedNames_) {
            std::cout << RED << "  ✗ " << name << RESET << "\n";
        }
        std::cout << std::flush;
    }

private:
    int passedCount_ = 0;
    std::vector<std::string> failedNames_;
};

// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners& listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    // Remove GoogleTest's default printer so output isn't duplicated, then install
    // our per-test listener. (To keep the default output too, skip this Release line.)
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new SegmenterTestListener);

    return RUN_ALL_TESTS();
}
