#include "segmenterFactory.hpp"
#include "segmenter.hpp"
#include <stdexcept>

//ThresholdSegmenterFactory: creates a ThresholdSegmenter using params from config
std::unique_ptr<SegmenterBase> ThresholdSegmenterFactory::create(const ConfigLoader& config) const {
    return std::make_unique<ThresholdSegmenter>(
        config.getParam("Threshold", "threshold"),
        config.getParam("Threshold", "maxValue"),
        config.getFeatureNames() //read once at construction - no config dependency during segment()
    );
}

//CannySegmenterFactory: creates a CannySegmenter using params from config
std::unique_ptr<SegmenterBase> CannySegmenterFactory::create(const ConfigLoader& config) const {
    return std::make_unique<CannySegmenter>(
        config.getParam("Canny", "lowThreshold"),
        config.getParam("Canny", "highThreshold"),
        config.getFeatureNames() //read once at construction - no config dependency during segment()
    );
}

//dispatcher: reads style name and calls the corresponding segmenter factory instance
std::unique_ptr<SegmenterBase> SegmenterFactory::create(const std::string& styleName, const ConfigLoader& config) {
    if (styleName == "Threshold") return ThresholdSegmenterFactory().create(config);
    if (styleName == "Canny")     return CannySegmenterFactory().create(config);
    throw std::invalid_argument("Unknown segmentation style: " + styleName);
}