#include "featureExtractor.hpp"

//builds and returns the feature lookup map for one extractFeatures call
//to add a new feature: add one entry here - the lambda captures whatever data source it needs
std::map<std::string, FeatureComputer> buildFeatureLookup(const cv::Mat& stats, const cv::Mat& centroids) { //stats and centroids are captured by reference from the caller so lambdas are generic (int i only)
    return {
        {"labelID", [](int i) -> FeatureValue {
            return i;
        }},
        {"Area", [&stats](int i) -> FeatureValue { 
            return static_cast<float>(stats.at<int>(i, cv::CC_STAT_AREA)); 
        }},
        {"Centroid", [&centroids](int i) -> FeatureValue {
            return cv::Point2d(centroids.at<double>(i, 0), centroids.at<double>(i, 1));
        }},
        {"BoundingBox", [&stats](int i) -> FeatureValue {
            return cv::Rect(
                stats.at<int>(i, cv::CC_STAT_LEFT),
                stats.at<int>(i, cv::CC_STAT_TOP),
                stats.at<int>(i, cv::CC_STAT_WIDTH),
                stats.at<int>(i, cv::CC_STAT_HEIGHT)
            );
        }},
        {"Density", [&stats](int i) -> FeatureValue {
            float area     = static_cast<float>(stats.at<int>(i, cv::CC_STAT_AREA));
            float bboxArea = static_cast<float>(
                stats.at<int>(i, cv::CC_STAT_WIDTH) * stats.at<int>(i, cv::CC_STAT_HEIGHT)
            );
            return (bboxArea > 0.0f) ? area / bboxArea : 0.0f; 
        }}
    };
}
