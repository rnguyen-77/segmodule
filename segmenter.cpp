#include "segmenter.hpp"
#include "featureExtractor.hpp"
#include <stdexcept>

std::map<std::string, std::map<std::string, double>> ConfigLoader::config_;
YAML::Node ConfigLoader::root_;

//implementation of configuration loading from YAML file
void ConfigLoader::loadConfig(const std::string& filename) {
    root_ = YAML::LoadFile(filename);
    config_.clear();

    if (!root_["Segmentation"]["activeStyle"] || !root_["Segmentation"]["activeStyle"].IsScalar()){
        throw std::runtime_error("activeStyle missing or not a string");
    }
    for (const auto& kv:root_["Segmentation"]["Parameters"]){ //iterate through top-level keys in YAML file
        const std::string style = kv.first.as<std::string>();
        if (style == "activeStyle") {
            continue;
        }
        const YAML::Node params = kv.second;
        if (!params.IsMap()){
            throw std::runtime_error("Style block must be a map: " + style);
        }
        for (const auto& p:params){ //iterate through parameters for each style
            const std::string paramName = p.first.as<std::string>();
            double paramValue = 0.0;
            try {
                paramValue = p.second.as<double>();
            } catch (const YAML::BadConversion&) {
                throw std::runtime_error("Expected number for " + style + ": " + paramName);
            }
        
            config_[style][paramName] = paramValue;
        }
    }
    const std::string active = getActiveStyle();
    if (!config_.contains(active)){
        throw std::runtime_error("activeStyle does not match any style block: " + active);
    }
}

//returns active segmentation style name from config
std::string ConfigLoader::getActiveStyle() {
    if (!root_["Segmentation"]["activeStyle"] || !root_["Segmentation"]["activeStyle"].IsScalar()){
        throw std::runtime_error("activeStyle missing or not a string");
    }
    return root_["Segmentation"]["activeStyle"].as<std::string>();
}

//returns parameter value for given segmentation style and parameter name
double ConfigLoader::getParam(const std::string& styleName, const std::string& paramName) {
    if (!config_.contains(styleName)) {
        throw std::runtime_error("Segmentation style not found in config: " + styleName);
    }
    if (!config_[styleName].contains(paramName)) {
        throw std::runtime_error("Parameter not found in config for style " + styleName + ": " + paramName);
    }
    return config_[styleName][paramName];
}

//reads feature names from the Features block in config and returns them as a vector of strings
std::vector<std::string> ConfigLoader::getFeatureNames() {
    if (!root_["Features"] || !root_["Features"].IsSequence()) {
        throw std::runtime_error("Features block missing or not a sequence in config");
    }

    std::vector<std::string> names;
    for (const auto& entry : root_["Features"]) {
        if (!entry["name"] || !entry["name"].IsScalar()) {
            throw std::runtime_error("Feature entry missing 'name' field");
        }
        names.push_back(entry["name"].as<std::string>());
    }
    return names;
}

//ObjectFeatures method implementations
void ObjectFeatures::set(const std::string& name, const FeatureValue& value) {
    features_[name] = value;
}

FeatureValue ObjectFeatures::get(const std::string& name) const {
    auto it = features_.find(name);
    if (it == features_.end()) {
        throw std::out_of_range("Feature not found: " + name);
    }
    return it->second;
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
    const std::vector<std::string> featureNames = ConfigLoader::getFeatureNames();

    cv::Mat labelMat, stats, centroids;
    int numComponents = cv::connectedComponentsWithStats(mask, labelMat, stats, centroids);

    labelImage = convertMatToALOG(labelMat); //placeholder until ALOG class is ready

    //build lookup map here so lambdas can capture stats/centroids from this scope
    const auto featureLookup = buildFeatureLookup(stats, centroids);

    for (int i = 1; i < numComponents; ++i) { //skip component 0 (background)
        ObjectFeatures obj;

        for (const auto& name : featureNames) {
            auto it = featureLookup.find(name);
            if (it != featureLookup.end()) {
                obj.set(name, it->second(i)); //call the generic lambda for this component
            }
        }

        objects.push_back(obj);
    }
}

