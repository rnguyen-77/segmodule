#include "segmenter.hpp"
#include <iostream>


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
       
        ALOG alogImage; //placeholder ALOG image - ideally imported from bagLoader module and stored for processing 
        const cv::Mat conversion = segmenter->convertALOGtoMat(alogImage); //convert ALOG image to OpenCV Mat format
        const cv::Mat segmented = segmenter->segment(conversion); //perform segmentation on converted image and return segmented mask
        const SegmentationResult result = segmenter->extractFeatures(segmented); //extract features from segmented mask and store in SegmentationResult struct


        cv::imshow("Original", img);
        cv::imshow("Segmented - " + styleName, segmented);
        cv::waitKey(0);

    } catch (const std::exception& ex) {
        std::cerr << "Segmentation error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
