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
//this keeps the interface independent of any specific library (OpenCV, ITK, bagLoader, etc.)
using FeatureComputer = std::function<FeatureValue(int i)>;
