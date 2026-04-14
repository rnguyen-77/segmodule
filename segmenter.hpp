#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <yaml-cpp/yaml.h>


//class to load and store configuration parameters for segmenters
class ConfigLoader {
public:
    static void loadConfig(const std::string& filename);
    static std::string getActiveStyle(); 
    static double getParam(const std::string& styleName, const std::string& paramName);

private:
    static std::map<std::string, std::map<std::string, double>> config_; //nested map to store parameters for each segmentation style
    static YAML::Node root_;
};

//enum class to define different segmentation styles
enum class SegmentationStyle {
    Threshold,
    Canny
};

//class wrapper for segmented objects
class ObjectFeatures {
public: //future utility functions
    int getArea() const { return area; }
private:
    int labelID;
    int area;
    cv::Rect bbox;
    cv::Point2d centroid;

    //placeholder metadata for future concept
    std::string category;
    bool isSignificant;
};

//struct to hold segmentation results
struct SegmentationResult {
    cv::Mat labelImage; //expected image/CT image output
    std::vector<ObjectFeatures> objects; //vector to store segmented objects - objects characterized by ObjectFeatures class
};

//base class for image segmenters
class SegmenterBase {
public:
    virtual ~SegmenterBase() = default; //virtual destructor to ensure proper cleanup of derived class objects through base class pointers
    virtual cv::Mat segment(const cv::Mat& input) const = 0; //pure virtual function to be implemented by derived classes 
protected:
    SegmentationResult extractFeatures(const cv::Mat& input) const; 
    cv::Mat convertALOGtoMat(const ALOG& alog) const; //placeholder concept: converts ALOG images (from bagLoader module) to OpenCV Mat format for processing
};

//
class CommonSegmenter : public SegmenterBase {
public:
    virtual cv::Mat segment(const cv::Mat& input) const = 0;
protected:
    cv::Mat toGray(const cv::Mat& input) const;
};

//derived class for thresholding segmentation
class ThresholdSegmenter : public CommonSegmenter {
public:
    ThresholdSegmenter(double threshold, double maxValue)
        : threshold_(threshold), maxValue_(maxValue) {}

    cv::Mat segment(const cv::Mat& input) const override;

private:
    double threshold_;
    double maxValue_;
};

//derived class for Canny edge detection segmentation
class CannySegmenter : public CommonSegmenter {
public:
    CannySegmenter(double lowThreshold, double highThreshold)
        : lowThreshold_(lowThreshold), highThreshold_(highThreshold) {}

    cv::Mat segment(const cv::Mat& input) const override;
  
private:
    double lowThreshold_;
    double highThreshold_;
};

//factory class: creates segmenter objects based on style enum
class SegmenterFactory {
public:
    static std::unique_ptr<SegmenterBase> create(SegmentationStyle style);
};

//function to parse string representation of segmentation style from YAML file
SegmentationStyle parseStyle(const std::string& style);