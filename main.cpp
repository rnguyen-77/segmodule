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
        ConfigLoader config(configPath); //construct and load config - throws on bad file or schema

        //get active segmentation style from config unless user overrides it
        const std::string styleName = styleOverride.empty() ? config.getActiveStyle() : styleOverride;
        //create segmenter object based on selected style using factory method
        const std::unique_ptr<SegmenterBase> segmenter = SegmenterFactory::create(styleName, config);
       
        ALOG alogImage = load(imagePath); //placeholder ALOG image - replace load logic once ALOG image class is defined
        std::vector<ObjectFeatures> objects;
        ALOG alogLabelImage; //placeholder ALOG image to hold segmentation output

        segmenter->segment(alogImage, alogLabelImage, objects); //segment image and extract features
    
        cv::imshow("Original", CommonSegmenter::convertALOGtoMat(alogImage)); //processing/display to be adjusted in the future 
        cv::imshow("Segmented - " + styleName, CommonSegmenter::convertALOGtoMat(alogLabelImage));
        cv::waitKey(0);

    } catch (const std::exception& ex) {
        std::cerr << "Segmentation error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
