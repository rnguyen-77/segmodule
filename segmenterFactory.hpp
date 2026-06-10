#pragma once

#include <memory>
#include <string>

class SegmenterBase; //forward declaration of base segmenter class
class ConfigLoader;  //forward declaration - full definition not needed in header

//abstract base factory class - derived factories override create() to build a specific segmenter
class SegmenterFactory {
public:
    virtual ~SegmenterFactory() = default;
    virtual std::unique_ptr<SegmenterBase> create(const ConfigLoader& config) const = 0;

    //static dispatcher: reads style name and delegates to the matching derived factory
    static std::unique_ptr<SegmenterFactory> factoryStyle(const std::string& styleName);
};

//derived factory for Threshold segmenter - owns creation logic for ThresholdSegmenter
class ThresholdSegmenterFactory : public SegmenterFactory {
public:
    std::unique_ptr<SegmenterBase> create(const ConfigLoader& config) const override;
};

//derived factory for Canny segmenter - owns creation logic for CannySegmenter
class CannySegmenterFactory : public SegmenterFactory {
public:
    std::unique_ptr<SegmenterBase> create(const ConfigLoader& config) const override;
};
