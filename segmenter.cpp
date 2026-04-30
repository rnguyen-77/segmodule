#include "segmenter.hpp"
#include "featureExtractor.hpp"
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

//reads feature names from the Features block in config and returns them as a vector of strings
std::vector<std::string> ConfigLoader::getFeatureNames() const {
    if (!root_["Features"] || !root_["Features"].IsSequence()) {
        throw std::runtime_error("Features block missing or not a sequence in config");
    }

    std::vector<std::string> names; //feature names stored in a vector
    for (const auto& entry : root_["Features"]) { //iterates through each entry under the Features block - assumes each type with auto due to potentially different types of features (e.g. numeric, string, boolean) 
        if (!entry["name"] || !entry["name"].IsScalar()) {
            throw std::runtime_error("Feature entry missing 'name' field");
        }
        names.push_back(entry["name"].as<std::string>()); //populates vector
    }
    return names;
}

//ObjectFeatures method implementations - placeholders
void ObjectFeatures::set(const std::string& name, const FeatureValue& value) {
    features_[name] = value;
}

FeatureValue ObjectFeatures::get(const std::string& name) const {
    auto it = features_.find(name); //lookup feature by name in map
    if (it == features_.end()) {
        throw std::out_of_range("Feature not found: " + name);
    }
    return it->second; //returns value from iterator if found
}

bool ObjectFeatures::has(const std::string& name) const {
    return features_.contains(name);
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
void ThresholdSegmenter::segment(const ALOG& alogInput, ALOG& labelImage, std::vector<ObjectFeatures>& objects) const {
    cv::Mat cvMat = convertALOGtoMat(alogInput);

    cv::Mat gray = toGray(cvMat);
    cv::Mat mask;
    cv::threshold(gray, mask, threshold_, maxValue_, cv::THRESH_BINARY);

    extractFeatures(mask, labelImage, objects);
}

//implementation of Canny edge detection segmentation using OpenCV's Canny function
void CannySegmenter::segment(const ALOG& alogInput, ALOG& labelImage, std::vector<ObjectFeatures>& objects) const {
    cv::Mat cvMat = convertALOGtoMat(alogInput);

    cv::Mat gray = toGray(cvMat);
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.0);
    cv::Mat edges;
    cv::Canny(blurred, edges, lowThreshold_, highThreshold_);

    extractFeatures(edges, labelImage, objects);
}

//feature extraction: runs connected component analysis then populates one ObjectFeatures per component
void CommonSegmenter::extractFeatures(const cv::Mat& mask, ALOG& labelImage, std::vector<ObjectFeatures>& objects) const {
    cv::Mat labelMat, stats, centroids;
    int numComponents = cv::connectedComponentsWithStats(mask, labelMat, stats, centroids);

    labelImage = convertMatToALOG(labelMat); //placeholder until ALOG class is ready

    //lookup map from feature name to lambda that computes that feature for any given component index
    const auto featureLookup = buildFeatureLookup(stats, centroids);

    //iterate through each component and populate ObjectFeatures based on feature names specified in config
    for (int i = 1; i < numComponents; ++i) { //skip component 0 (background)
        ObjectFeatures obj;

        for (const auto& name : featureNames_) { //iterate through feature names specified in config 
            auto it = featureLookup.find(name); //lookup lambda for feature name
            if (it != featureLookup.end()) {
                obj.set(name, it->second(i)); //call the lambda with the current component index to compute the feature value
            }
        }
        objects.push_back(obj); //populates object vector with computed features for each component
    }
}

