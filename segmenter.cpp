#include "segmenter.hpp"
#include <stdexcept>

// 
std::map<std::string, std::map<std::string, double>> ConfigLoader::config_;
YAML::Node ConfigLoader::root_;

//
void ConfigLoader::loadConfig(const std::string& filename) {
    root_ = YAML::LoadFile(filename);
    config_.clear();

    if (!root_["activeStyle"] || !root_["activeStyle"].IsScalar()){
        throw std::runtime_error("activeStyle missing or not a string");
    }
    for (const auto& kv:root_){
        const std::string style = kv.first.as<std::string>();
        if (style == "activeStyle") {
            continue;
        }
        const YAML::Node params = kv.second;
        if (!params.IsMap()){
            throw std::runtime_error("Style block must be a map: " + style);
        }
        for (const auto& p:params){
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

//
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

//base class implementation for common functionality like converting input image to grayscale
cv::Mat SegmenterBase::toGray(const cv::Mat& input) const {
    if (input.empty()) {
        throw std::invalid_argument("Input image is empty.");
    }
    if (input.channels() == 1) {
        return input;
    }
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

//
SegmentationResult SegmenterBase::segmentWithFeatures(const cv::Mat& input) const {
    if (input.empty()) {
        throw std::invalid_argument("Input image is empty.");
    }

    cv::Mat labelImage;
    cv::Mat stats;
    cv::Mat centroids;
    cv::connectedComponentsWithStats(input, labelImage, stats, centroids);

    SegmentationResult result;
    result.labels = labelImage;
    result.objects.clear();

    const int numLabels = stats.rows; // label 0 is background
    for (int label = 1; label < numLabels; ++label) {
        ObjectFeatures obj{};
        obj.labelID = label;

        obj.area = stats.at<int>(label, cv::CC_STAT_AREA);

        const int left   = stats.at<int>(label, cv::CC_STAT_LEFT);
        const int top    = stats.at<int>(label, cv::CC_STAT_TOP);
        const int width  = stats.at<int>(label, cv::CC_STAT_WIDTH);
        const int height = stats.at<int>(label, cv::CC_STAT_HEIGHT);
        obj.bbox = cv::Rect(left, top, width, height);

        obj.centroid = cv::Point2d(
            centroids.at<double>(label, 0), // x
            centroids.at<double>(label, 1)  // y
        );
        result.objects.push_back(obj);
    }
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
