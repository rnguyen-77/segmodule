#include "segmenter.hpp"

int main(int argc, char* argv[]) {
    ConfigLoader::loadConfig("../segmenter_config.json"); //load configuration parameters from JSON file

    const string path = "/mnt/netapp/SECURITY/users/rnguyen/segModule/card.jpg";
    const Mat img = imread(path);
    if (img.empty()) {
        cerr << "Failed to load image: " << path << '\n';
        return 1;
    }

    //get active segmentation style from config
    const string styleName = ConfigLoader::getActiveStyle();
    //return enum value corresponding to activeStyle string
    const SegmentationStyle style = parseStyle(styleName);

    //create segmenter object based on selected style using factory pattern
    const unique_ptr<SegmenterBase> segmenter = SegmenterFactory::create(style);
    //segment the image using the created segmenter object
    const Mat segmented = segmenter->segment(img);

    imshow("Original", img);
    imshow("Segmented - " + styleName, segmented);
    waitKey(0);
    return 0;
}
