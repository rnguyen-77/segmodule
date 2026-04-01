#pragma once

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace cv;
using namespace std;
using json = nlohmann::json;

//class to load and store configuration parameters for segmenters
class ConfigLoader {
public:
    static void loadConfig(const string& filename);
    static string getActiveStyle();
    static double getParam(const string& styleName, const string& paramName);

private:
    static map<string, map<string, double>> config_; //nested map to store parameters for each segmentation style
    static json jsonRoot; //json object to store entire configuration from JSON file
};

//enum class to define different segmentation styles
enum class SegmentationStyle {
    Threshold,
    Canny
};

//base class for image segmenters
class SegmenterBase {
public:
    virtual ~SegmenterBase() = default; //virtual destructor to ensure proper cleanup of derived class objects through base class pointers
    virtual Mat segment(const Mat& input) const = 0; //pure virtual function to be implemented by derived classes

protected:
    Mat toGray(const Mat& input) const;
};

//derived class for thresholding segmentation
class ThresholdSegmenter : public SegmenterBase {
public:
    ThresholdSegmenter(double threshold, double maxValue)
        : threshold_(threshold), maxValue_(maxValue) {}

    Mat segment(const Mat& input) const override;

private:
    double threshold_;
    double maxValue_;
};

//derived class for Canny edge detection segmentation
class CannySegmenter : public SegmenterBase {
public:
    CannySegmenter(double lowThreshold, double highThreshold)
        : lowThreshold_(lowThreshold), highThreshold_(highThreshold) {}

    Mat segment(const Mat& input) const override;

private:
    double lowThreshold_;
    double highThreshold_;
};

//factory class: creates segmenter objects based on style enum
class SegmenterFactory {
public:
    static unique_ptr<SegmenterBase> create(SegmentationStyle style);
};

//function to parse string representation of segmentation style from JSON file
SegmentationStyle parseStyle(const string& style);