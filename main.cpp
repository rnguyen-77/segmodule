#include "segmenter.hpp"
#include "segmenterFactory.hpp"
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
        //create segmenter object based on selected style using factory method
        const std::unique_ptr<SegmenterBase> segmenter = SegmenterFactory::create(styleName); 
       
        ALOG alogImage; //placeholder ALOG image - ideally imported from bagLoader module and stored for processing 
        const SegmentationResult result = segmenter->process(alogImage); //main processing function: converts ALOG image to Mat, applies segmentation, and extracts features
    
        cv::imshow("Original", img);
        cv::imshow("Segmented - " + styleName, result.labelImage);
        cv::waitKey(0);

    } catch (const std::exception& ex) {
        std::cerr << "Segmentation error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
