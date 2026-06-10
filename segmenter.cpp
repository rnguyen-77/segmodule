#include "segmenter.hpp"
#include <stdexcept>

//constructor that loads config from file for any instance
ConfigLoader::ConfigLoader(const std::string& filename) {
    loadConfig(filename);
}

//loads and parses configuration from YAML file 
void ConfigLoader::loadConfig(const std::string& filename) {
    root_ = YAML::LoadFile(filename); 
    config_.clear();

    //validate expected structure and specified segmentation style
    if (!root_["Segmentation"]["activeStyle"] || !root_["Segmentation"]["activeStyle"].IsScalar()){
        throw std::runtime_error("activeStyle missing or not a string");
    }
    activeStyle_ = root_["Segmentation"]["activeStyle"].as<std::string>(); //stores active style name for later retrieval by segmenter factories

    for (const auto& kv:root_["Segmentation"]["Parameters"]){ //iterate through each style block under Parameters
        const std::string style = kv.first.as<std::string>(); //style name (e.g. "Threshold", "Canny")
        if (style == "activeStyle") {
            continue;
        }
        const YAML::Node params = kv.second; //parameters for this style (e.g. threshold value, maxValue for Threshold)
        if (!params.IsMap()){ //validate that parameters are key-value pairs
            throw std::runtime_error("Style block must be a map: " + style);
        }
        for (const auto& p:params){ //iterate through parameters for each style
            const std::string paramName = p.first.as<std::string>(); //parameter name (e.g. "threshold", "maxValue")
            double paramValue = 0.0; //placeholder - currently only supporting numeric parameters for simplicity - can be extended to support other types as needed
            try {
                paramValue = p.second.as<double>(); //parse param value as double - will throw if not a number - extendable to support other types if needed
            } catch (const YAML::BadConversion&) {
                throw std::runtime_error("Expected number for " + style + ": " + paramName);
            }
        
            config_[style][paramName] = paramValue; //store in nested map for later retrieval by segmenter factories
        }
    }
    if (!config_.contains(activeStyle_)) { //validate that activeStyle specified in config matches one of the style blocks under Parameters
        throw std::runtime_error("activeStyle does not match any style block: " + activeStyle_);
    }
}

//returns active segmentation style name from config for segmenter factory to determine which segmenter to construct
std::string ConfigLoader::getActiveStyle() const {
    return activeStyle_;
}

//returns parameter value for given segmentation style and parameter name
double ConfigLoader::getParam(const std::string& styleName, const std::string& paramName) const {
    if (!config_.contains(styleName)) {
        throw std::runtime_error("Segmentation style not found in config: " + styleName);
    }
    if (!config_[styleName].contains(paramName)) {
        throw std::runtime_error("Parameter not found in config for style " + styleName + ": " + paramName);
    }
    return config_[styleName][paramName];
}

//Objects feature map method implementations
void Objects::setFeature(const std::string& name, double value) {
    features_[name] = value;
}

double Objects::getFeature(const std::string& name) const {
    auto it = features_.find(name);
    if (it == features_.end()) {
        throw std::out_of_range("Feature not found: " + name);
    }
    return it->second;
}

bool Objects::hasFeature(const std::string& name) const {
    return features_.contains(name);
}

//ObjectCollection method implementations
void ObjectCollection::add(Objects obj) {
    objects_.push_back(std::move(obj));
}

void ObjectCollection::clear() {
    objects_.clear();
}

const std::vector<Objects>& ObjectCollection::all() const {
    return objects_;
}

std::size_t ObjectCollection::size() const {
    return objects_.size();
}

//ALOG image class adapter
cv::Mat CommonSegmenter::convertALOGtoMat(const ALOG& alog) const {
    //placeholder implementation 
    return cv::Mat();
}

ALOG CommonSegmenter::convertMatToALOG(const cv::Mat& mat) const {
    //placeholder implementation 
    return ALOG();
}

//helper function for common preprocessing step to convert input image to grayscale if needed
cv::Mat CommonSegmenter::toGray(const cv::Mat& input) const {
    if (input.channels() == 1) {
        return input;
    }
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

//implementation of thresholding segmentation using OpenCV's threshold function
void ThresholdSegmenter::segment(const ALOG& alogInput, ALOG& labelImage, ObjectCollection& objects) const {
    cv::Mat cvMat = convertALOGtoMat(alogInput);

    cv::Mat gray = toGray(cvMat);
    cv::Mat mask;
    cv::threshold(gray, mask, threshold_, maxValue_, cv::THRESH_BINARY);

    findObjects(mask, labelImage, objects);
}

//implementation of Canny edge detection segmentation using OpenCV's Canny function
void CannySegmenter::segment(const ALOG& alogInput, ALOG& labelImage, ObjectCollection& objects) const {
    cv::Mat cvMat = convertALOGtoMat(alogInput);

    cv::Mat gray = toGray(cvMat);
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.0);
    cv::Mat edges;
    cv::Canny(blurred, edges, lowThreshold_, highThreshold_);

    findObjects(edges, labelImage, objects);
}

//runs connected component analysis on the binary mask to identify distinct objects and produce the labeled image
void CommonSegmenter::findObjects(const cv::Mat& mask, ALOG& labelImage, ObjectCollection& objects) const {
    cv::Mat labelMat, stats, centroids;
    int numComponents = cv::connectedComponentsWithStats(mask, labelMat, stats, centroids);

    labelImage = convertMatToALOG(labelMat); //placeholder until ALOG class is ready

    //label 0 is the background component - skip it and start from 1
    for (int i = 1; i < numComponents; ++i) {
        Objects obj(i);
        obj.setFeature("Area",       static_cast<double>(stats.at<int>(i, cv::CC_STAT_AREA)));
        obj.setFeature("Centroid_x", centroids.at<double>(i, 0));
        obj.setFeature("Centroid_y", centroids.at<double>(i, 1));

        objects.add(std::move(obj));
    }
}

