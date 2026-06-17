#include "segmenterFactory.hpp"
#include "segmenter.hpp"
#include <stdexcept>

//ThresholdSegmenterFactory: creates a ThresholdSegmenter using params from config
std::unique_ptr<SegmenterBase> ThresholdSegmenterFactory::create(const ConfigLoader& config) const {
    return std::make_unique<ThresholdSegmenter>(
        config.getParam("Threshold", "threshold"),
        config.getParam("Threshold", "maxValue")
    );
}

//CannySegmenterFactory: creates a CannySegmenter using params from config
std::unique_ptr<SegmenterBase> CannySegmenterFactory::create(const ConfigLoader& config) const {
    return std::make_unique<CannySegmenter>(
        config.getParam("Canny", "lowThreshold"),
        config.getParam("Canny", "highThreshold")
    );
}

//static selector: returns a derived factory object for the given style name
//caller then invokes factory->create(config) to build the actual segmenter
std::unique_ptr<SegmenterFactoryBase> SegmenterFactoryBase::factoryStyle(const std::string& styleName) {
    if (styleName == "Threshold") return std::make_unique<ThresholdSegmenterFactory>();
    if (styleName == "Canny")     return std::make_unique<CannySegmenterFactory>();
    throw std::invalid_argument("Unknown segmentation style: " + styleName);
}