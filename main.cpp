#include "segmenter.hpp"
#include "segmenterFactory.hpp"
#include <iostream>


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [config.yaml] [image_path]\n";
        return 1;
    }

    const std::string configPath = argv[1];
    const std::string imagePath = argv[2];

    try {
        ConfigLoader config(configPath); //construct and load config - throws on bad file or schema

        //get active segmentation style from config
        const std::string styleName = config.getActiveStyle();
        //two-step factory: first get the derived factory for the style, then create the segmenter
        const std::unique_ptr<SegmenterFactoryBase> factory = SegmenterFactoryBase::factoryStyle(styleName);
        const std::unique_ptr<SegmenterBase> segmenter = factory->create(config);

        std::cout << "Loading bag: " << imagePath << " ...\n";
        SegImage alogImage = load(imagePath); //loads a .mi.gz bag via the bagLoader module
        std::cout << "Loaded slice: " << alogImage.mat().cols << " x " << alogImage.mat().rows
                  << " (style: " << styleName << ")\n";

        Objects objects;
        SegImage alogLabelImage; //SegImage image to hold segmentation output

        segmenter->segment(alogImage, alogLabelImage, objects); //segment image and extract features
        std::cout << "Segmentation complete: " << objects.size() << " object(s) found.\n";

    } catch (const std::exception& ex) {
        std::cerr << "Segmentation error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}