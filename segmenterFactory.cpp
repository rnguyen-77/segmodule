#include "segmenterFactory.hpp"
#include "segmenter.hpp"
#include <stdexcept>

//ThresholdSegmenterFactory: creates a ThresholdSegmenter using params from config
std::unique_ptr<SegmenterBase> ThresholdSegmenterFactory::create() const {
    return std::make_unique<ThresholdSegmenter>(
        ConfigLoader::getParam("Threshold", "threshold"),
        ConfigLoader::getParam("Threshold", "maxValue")
    );
}

//CannySegmenterFactory: creates a CannySegmenter using params from config
std::unique_ptr<SegmenterBase> CannySegmenterFactory::create() const {
    return std::make_unique<CannySegmenter>(
        ConfigLoader::getParam("Canny", "lowThreshold"),
        ConfigLoader::getParam("Canny", "highThreshold")
    );
}

//dispatcher: reads style name and calls the corresponding segmenter factory instance
std::unique_ptr<SegmenterBase> SegmenterFactory::create(const std::string& styleName) {
    if (styleName == "Threshold") return ThresholdSegmenterFactory().create();
    if (styleName == "Canny")     return CannySegmenterFactory().create();
    throw std::invalid_argument("Unknown segmentation style: " + styleName);
}