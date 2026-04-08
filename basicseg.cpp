#include "segmenter.hpp"
#include <iostream>

namespace {
    cv::Mat createAnnotatedView(const cv::Mat& image, const SegmentationResult& result) {
    cv::Mat annotated;
    if (image.channels() == 1) {
        cv::cvtColor(image, annotated, cv::COLOR_GRAY2BGR);
    } else {
        annotated = image.clone();
    }
    for (const auto& object : result.objects) {
        cv::rectangle(annotated, object.bbox, cv::Scalar(0, 255, 0), 2);
        cv::circle(annotated, object.centroid, 4, cv::Scalar(0, 0, 255), cv::FILLED);

        const std::string labelText = "ID " + std::to_string(object.labelID);
        const cv::Point textOrigin(
            object.bbox.x,
            std::max(20, object.bbox.y - 8)
        );
        cv::putText(
            annotated,
            labelText,
            textOrigin,
            cv::FONT_HERSHEY_SIMPLEX,
            0.6,
            cv::Scalar(255, 0, 0),
            2
        );
    }
    return annotated;
}
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [config.yaml] [image_path] [style_override]\n";
        return 1;
    }

    const std::string configPath = argv[1];
    const std::string imagePath = argv[2];
    const std::string styleOverride = (argc > 3) ? argv[3] : "";

    try {
        ConfigLoader::loadConfig(configPath); //load configuration parameters from YAML file
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    const cv::Mat img = cv::imread(imagePath);
    if (img.empty()) {
        std::cerr << "Failed to load image: " << imagePath << '\n';
        return 1;
    }

    try {
        //get active segmentation style from config unless user overrides it
        const std::string styleName = styleOverride.empty() ? ConfigLoader::getActiveStyle() : styleOverride;
        //return enum value corresponding to activeStyle string
        const SegmentationStyle style = parseStyle(styleName);

        //create segmenter object based on selected style using factory pattern
        const std::unique_ptr<SegmenterBase> segmenter = SegmenterFactory::create(style);
        //segment the image using the created segmenter object
        const cv::Mat mask = segmenter->segment(img);
        const SegmentationResult result = segmenter->segmentWithFeatures(mask);
        const cv::Mat annotatedMask = createAnnotatedView(mask, result);

        std::cout << "Objects found: " << result.objects.size() << '\n';
        for (const auto& o : result.objects) {
            std::cout << "Label " << o.labelID
                    << " area=" << o.area
                    << " bbox=(" << o.bbox.x << "," << o.bbox.y
                    << "," << o.bbox.width << "," << o.bbox.height << ")"
                    << " centroid=(" << o.centroid.x << "," << o.centroid.y << ")\n";
}

        cv::imshow("Original", img);
        cv::imshow("Segmented - " + styleName, mask);
        cv::imshow("Annotated Mask - " + styleName, annotatedMask);
        cv::waitKey(0);

    } catch (const std::exception& ex) {
        std::cerr << "Segmentation error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
