#include "segmenter.hpp"

// Static member definitions — must live in exactly one .cpp file
map<string, map<string, double>> ConfigLoader::config_;
json ConfigLoader::jsonRoot;

void ConfigLoader::loadConfig(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open config file: " + filename);
    }

    file >> jsonRoot;
    for (auto& [style, params] : jsonRoot.items()) { //iterates through parameters for segmentaion styles defined in JSON file
        if (!params.is_object()) {
            continue; //skips non-object entries like activeStyle
        }
        for (auto& [paramName, paramValue] : params.items()) { //iterates through parameters values for each parameter
            if (!paramValue.is_number()) { //ensures all parameters are numeric for simplicity
                throw runtime_error("Expected number for " + style + "." + paramName);
            }
            config_[style][paramName] = paramValue;
        }
    }
}

string ConfigLoader::getActiveStyle() {
    if (!jsonRoot.contains("activeStyle")) {
        throw runtime_error("Active segmentation style not found in config.");
    }
    return jsonRoot["activeStyle"].get<string>();
}

double ConfigLoader::getParam(const string& styleName, const string& paramName) {
    if (!config_.contains(styleName)) {
        throw runtime_error("Segmentation style not found in config: " + styleName);
    }
    if (!config_[styleName].contains(paramName)) {
        throw runtime_error("Parameter not found in config for style " + styleName + ": " + paramName);
    }
    return config_[styleName][paramName];
}

Mat SegmenterBase::toGray(const Mat& input) const {
    if (input.empty()) {
        throw invalid_argument("Input image is empty.");
    }
    if (input.channels() == 1) {
        return input;
    }
    Mat gray;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    return gray;
}

Mat ThresholdSegmenter::segment(const Mat& input) const {
    Mat gray = toGray(input);
    Mat mask;
    threshold(gray, mask, threshold_, maxValue_, THRESH_BINARY);
    return mask;
}

Mat CannySegmenter::segment(const Mat& input) const {
    Mat gray = toGray(input);
    Mat blurred;
    GaussianBlur(gray, blurred, Size(5, 5), 1.0);
    Mat edges;
    Canny(blurred, edges, lowThreshold_, highThreshold_);
    return edges;
}

SegmentationStyle parseStyle(const string& style) {
    if (style == "Threshold") return SegmentationStyle::Threshold;
    if (style == "Canny") return SegmentationStyle::Canny;
    throw invalid_argument("Unknown activeStyle in config: " + style);
}

unique_ptr<SegmenterBase> SegmenterFactory::create(SegmentationStyle style) {
    switch (style) {
        case SegmentationStyle::Threshold:
            return make_unique<ThresholdSegmenter>(
                ConfigLoader::getParam("Threshold", "threshold"),
                ConfigLoader::getParam("Threshold", "maxValue")
            );
        case SegmentationStyle::Canny:
            return make_unique<CannySegmenter>(
                ConfigLoader::getParam("Canny", "lowThreshold"),
                ConfigLoader::getParam("Canny", "highThreshold")
            );
        default:
            throw invalid_argument("Unknown segmentation style.");
    }
}
