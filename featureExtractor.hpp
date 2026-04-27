#pragma once

#include <functional>
#include <map>
#include <string>
#include <variant>

#include <opencv2/core.hpp>

//type alias for all possible feature value types - add new types here as features expand
using FeatureValue = std::variant<int, float, double, cv::Point2d, cv::Rect, std::string, bool>;

//generic type alias for a feature computation function:
//takes only a component index - each lambda captures its own data sources via closure
using FeatureComputer = std::function<FeatureValue(int i)>;

//builds a map of feature name -> FeatureComputer for one extractFeatures call
//stats and centroids are captured by reference inside each lambda - add new parameters here
//when adding a feature that needs a different data source (e.g. custom metadata struct)
std::map<std::string, FeatureComputer> buildFeatureLookup(const cv::Mat& stats, const cv::Mat& centroids);
