#pragma once

#include <map>
#include <stdexcept>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <yaml-cpp/yaml.h>



//class to load and store configuration parameters for segmenters
class ConfigLoader {
public:
    explicit ConfigLoader(const std::string& filename); //constructor that loads config from file
    std::string getActiveStyle() const; //returns active segmentation style name from config
    double getParam(const std::string& styleName, const std::string& paramName) const; //returns parameter value for given segmentation style and parameter name
private:
    void loadConfig(const std::string& filename);
    std::map<std::string, std::map<std::string, double>> config_; //nested map to store parameters for each segmentation style
    YAML::Node root_;
    std::string activeStyle_;
};

//class wrapper for segmented objects found by connectedComponentsWithStats
class Objects {
public:
    explicit Objects(int label) : label_(label) {}
    int label() const { return label_; }  //connected component label index (identifier, not a computed feature)

    void setFeature(const std::string& name, double value);  //store a computed feature by name
    double getFeature(const std::string& name) const;        //retrieve a feature value by name
    bool hasFeature(const std::string& name) const;          //check if a feature was computed

    const std::map<std::string, double>& features() const { return features_; } //read-only access to all features

private:
    int label_;
    std::map<std::string, double> features_;  //named features populated during segmentation (e.g. "area", "centroid_x", "centroid_y")
};

//manages a collection of Objects found during segmentation
class ObjectCollection {
public:
    void add(Objects obj);                    //add a single object to the collection
    void clear();                             //remove all objects (e.g. before reprocessing)
    const std::vector<Objects>& all() const;  //read-only access to all objects
    std::size_t size() const;
private:
    std::vector<Objects> objects_;
};

//base class for image segmenters
class SegmenterBase {
public:
    virtual ~SegmenterBase() = default; //virtual destructor to ensure proper cleanup of derived class objects through base class pointers
    virtual void segment(const ALOG& alogInput, ALOG& labelImage, ObjectCollection& objects) const = 0; //pure virtual function to be implemented by derived classes
};

//derived class for common functions shared by multiple segmentation styles (e.g. grayscale conversion)
class CommonSegmenter : public SegmenterBase {
protected:
    cv::Mat toGray(const cv::Mat& input) const;
    void findObjects(const cv::Mat& input, ALOG& labelImage, ObjectCollection& objects) const;
    cv::Mat convertALOGtoMat(const ALOG& alog) const; //placeholder concept: converts ALOG images (from bagLoader module) to OpenCV Mat format for processing
    ALOG convertMatToALOG(const cv::Mat& mat) const; //placeholder concept: converts processed OpenCV Mat back to ALOG format for output
};

//derived class for thresholding segmentation
class ThresholdSegmenter : public CommonSegmenter {
public:
    ThresholdSegmenter(double threshold, double maxValue)
        : threshold_(threshold), maxValue_(maxValue) {}

    void segment(const ALOG& alogInput, ALOG& labelImage, ObjectCollection& objects) const override;

private:
    double threshold_;
    double maxValue_;
};

//derived class for Canny edge detection segmentation
class CannySegmenter : public CommonSegmenter {
public:
    CannySegmenter(double lowThreshold, double highThreshold)
        : lowThreshold_(lowThreshold), highThreshold_(highThreshold) {}

    void segment(const ALOG& alogInput, ALOG& labelImage, ObjectCollection& objects) const override;

private:
    double lowThreshold_;
    double highThreshold_;
};
