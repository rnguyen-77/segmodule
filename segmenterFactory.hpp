#pragma once

#include <memory>
#include <string>

class SegmenterBase; //forward declaration of base segmenter class
class ConfigLoader;  //forward declaration - full definition not needed in header

//factory class for Threshold segmenter - owns creation logic for ThresholdSegmenter
class ThresholdSegmenterFactory {
public:
    std::unique_ptr<SegmenterBase> create(const ConfigLoader& config) const;
};

//factory class for Canny segmenter - owns creation logic for CannySegmenter
class CannySegmenterFactory {
public:
    std::unique_ptr<SegmenterBase> create(const ConfigLoader& config) const;
};

//dispatcher: reads style name from config and calls the corresponding segmenter factory
//to add a new segmenter: add a new factory class above and one if-line in the dispatcher
class SegmenterFactory {
public:
    static std::unique_ptr<SegmenterBase> create(const std::string& styleName, const ConfigLoader& config);
};