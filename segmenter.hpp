#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <yaml-cpp/yaml.h>

#include "featureExtractor.hpp" 


//class to load and store configuration parameters for segmenters
//construct with a filename - object is always fully loaded after construction
class ConfigLoader {
public:
    explicit ConfigLoader(const std::string& filename); //loads config immediately on construction
    std::string getActiveStyle() const; 
    double getParam(const std::string& styleName, const std::string& paramName) const;
    std::vector<std::string> getFeatureNames() const;
private:
    void loadConfig(const std::string& filename);
    std::map<std::string, std::map<std::string, double>> config_; //nested map to store parameters for each segmentation style
    YAML::Node root_;
    std::string activeStyle_;
};

//class wrapper for segmented objects
class ObjectFeatures {
public:
    void set(const std::string& name, const FeatureValue& value);
    FeatureValue get(const std::string& name) const;
    bool has(const std::string& name) const;
private:
    std::map<std::string, FeatureValue> features_;
};

//base class for image segmenters
class SegmenterBase {
public:
    virtual ~SegmenterBase() = default; //virtual destructor to ensure proper cleanup of derived class objects through base class pointers
    virtual void segment(const ALOG& alogInput, ALOG& alogOutput, std::vector<ObjectFeatures>& objects) const = 0; //pure virtual function to be implemented by derived classes
};

//derived class for common functions shared by multiple segmentation styles (e.g. grayscale conversion)
class CommonSegmenter : public SegmenterBase {
protected:
    explicit CommonSegmenter(std::vector<std::string> featureNames)
        : featureNames_(std::move(featureNames)) {}

    cv::Mat toGray(const cv::Mat& input) const;
    void extractFeatures(const cv::Mat& input, ALOG& labelImage, std::vector<ObjectFeatures>& objects) const;
    cv::Mat convertALOGtoMat(const ALOG& alog) const; //placeholder concept: converts ALOG images (from bagLoader module) to OpenCV Mat format for processing
    ALOG convertMatToALOG(const cv::Mat& mat) const; //placeholder concept: converts processed OpenCV Mat back to ALOG format for output

private:
    std::vector<std::string> featureNames_; //feature names set at construction time - no config dependency during processing
};

//derived class for thresholding segmentation
class ThresholdSegmenter : public CommonSegmenter {
public:
    ThresholdSegmenter(double threshold, double maxValue, std::vector<std::string> featureNames)
        : CommonSegmenter(std::move(featureNames)), threshold_(threshold), maxValue_(maxValue) {}

    void segment(const ALOG& alogInput, ALOG& labelImage, std::vector<ObjectFeatures>& objects) const override;

private:
    double threshold_;
    double maxValue_;
};

//derived class for Canny edge detection segmentation
class CannySegmenter : public CommonSegmenter {
public:
    CannySegmenter(double lowThreshold, double highThreshold, std::vector<std::string> featureNames)
        : CommonSegmenter(std::move(featureNames)), lowThreshold_(lowThreshold), highThreshold_(highThreshold) {}

    void segment(const ALOG& alogInput, ALOG& labelImage, std::vector<ObjectFeatures>& objects) const override;
  
private:
    double lowThreshold_;
    double highThreshold_;
};
