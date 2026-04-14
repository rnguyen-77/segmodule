#include "segmenter.hpp"
#include <stdexcept>

//
std::map<std::string, std::map<std::string, double>> ConfigLoader::config_;
YAML::Node ConfigLoader::root_;

//implementation of configuration loading from YAML file
void ConfigLoader::loadConfig(const std::string& filename) {
    root_ = YAML::LoadFile(filename);
    config_.clear();

    if (!root_["activeStyle"] || !root_["activeStyle"].IsScalar()){
        throw std::runtime_error("activeStyle missing or not a string");
    }
    for (const auto& kv:root_){ //iterate through top-level keys in YAML file
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
    if (!root_["activeStyle"] || !root_ || !root_["activeStyle"].IsScalar()){
        throw std::runtime_error("activeStyle missing or not a string");
    }
    return root_["activeStyle"].as<std::string>();
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

//ALOG image class adapter
cv::Mat SegmenterBase::convertALOGtoMat(const ALOG& alog) const {
    //placeholder implementation 
    return cv::Mat();
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

//feature extraction concept - more complex feature extraction would be contained and imported from feature module in future
SegmentationResult SegmenterBase::extractFeatures(const cv::Mat& input) const {
    SegmentationResult result;

    //simple placeholder logic 
    cv::Mat output;
    cv::Mat stats;
    cv::Mat centroids;
    cv::connectedComponentsWithStats(input, output, stats, centroids);

    result.labelImage = output;
    result.objects.clear();

    //logic to populate object vector would be contained below

    return result;
}

//implementation of thresholding segmentation using OpenCV's threshold function
cv::Mat ThresholdSegmenter::segment(const cv::Mat& input) const {
    cv::Mat gray = toGray(input);
    cv::Mat mask;
    cv::threshold(gray, mask, threshold_, maxValue_, cv::THRESH_BINARY);
    return mask;
}

//implementation of Canny edge detection segmentation using OpenCV's Canny function
cv::Mat CannySegmenter::segment(const cv::Mat& input) const {
    cv::Mat gray = toGray(input);
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.0);
    cv::Mat edges;
    cv::Canny(blurred, edges, lowThreshold_, highThreshold_);
    return edges;
}

//helper function to parse active segmentation style string from config and return corresponding enum value
SegmentationStyle parseStyle(const std::string& style) {
    if (style == "Threshold") return SegmentationStyle::Threshold;
    if (style == "Canny") return SegmentationStyle::Canny;
    throw std::invalid_argument("Unknown activeStyle in config: " + style);
}

//factory method implementation to create segmenter object based on selected segmentation style
std::unique_ptr<SegmenterBase> SegmenterFactory::create(SegmentationStyle style) {
    switch (style) {
        case SegmentationStyle::Threshold:
            return std::make_unique<ThresholdSegmenter>(
                ConfigLoader::getParam("Threshold", "threshold"),
                ConfigLoader::getParam("Threshold", "maxValue")
            );
        case SegmentationStyle::Canny:
            return std::make_unique<CannySegmenter>(
                ConfigLoader::getParam("Canny", "lowThreshold"),
                ConfigLoader::getParam("Canny", "highThreshold")
            );
        default:
            throw std::invalid_argument("Unknown segmentation style.");
    }
}
