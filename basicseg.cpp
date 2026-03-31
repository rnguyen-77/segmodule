
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <memory>

#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace cv;
using namespace std;
using json = nlohmann::json;

class ConfigLoader { //class to load and store configuration parameters for segmenters
public:
    static void loadConfig(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Failed to open config file: " << filename << '\n';
            return;
        }
        json j;
        file >> j;
        file >> jsonRoot_;

        for (auto& [style, params] : j.items()) {
            for (auto& [paramName, paramValue] : params.items()) {
                config_[style][paramName] = paramValue;
            }
        }
    }
    static string getActiveStyle() {
        if (!jsonRoot_.contains("activeStyle")) {
            throw runtime_error("Active segmentation style not found in config.");
        }
        return jsonRoot_["activeStyle"].get<string>();
    }
    static double getParam(const string& styleName, const string& paramName) {
        if (config_.find(styleName) == config_.end()){
            throw runtime_error("Segmentation style not found in config: " + styleName);
        }   
        if (config_[styleName].find(paramName) == config_[styleName].end()) {
            throw runtime_error("Parameter not found in config for style " + styleName + ": " + paramName);
        }
        return config_[styleName][paramName];   
    }
private: 
    static map<string, map<string,double> >config_; //nested map to store parameters for each segmentation style
    static json jsonRoot_;
};

map<string, map<string, double> > ConfigLoader::config_; //initialize map
json ConfigLoader::jsonRoot_; //initialize json root

enum class SegmentationStyle {
    Threshold,
    Canny
    //insert additional styles here
};

SegmentationStyle parseStyle(const string& style) {
    if (style == "Threshold") return SegmentationStyle::Threshold;
    if (style == "Canny") return SegmentationStyle::Canny;
    throw invalid_argument("Unknown activeStyle in config: " + style);
}

class SegmenterBase {
public:
    virtual ~SegmenterBase() = default;
    virtual Mat segment(const Mat& input) const = 0;
    //virtual methods on basis of segmentation styles 
};

class ThresholdSegmenter : public SegmenterBase {
public:
    //constructor with default(necessary) parameters for thresholding
    //ThresholdSegmenter : initialize object
    //Mat segment : segment function
    //string name : name function
private:
    //stored values
    //untouchable to user
    Mat toGray(const Mat& input) const {
      //helper function
      //calls segment function
    }
    double threshold_;
    double maxValue_;
};

class CannySegmenter : public SegmenterBase {
public:
    //same logic as ThresholdSegmenter but for Canny edge detection
    /*
    CannySegmenter
    Mat segment
    string name
    */

private:
    Mat toGray(const Mat& input) const {
    }
    double lowThreshold_;
    double highThreshold_;
    //if segmenters share some helper functions consider creating a base class with common functionality to avoid code duplication and promote reusability
    //protected access specifier allows derived classes to use helper functions while keeping them hidden from users of segmenter objects
};

//factory class to create segmenter objects based on user input
class SegmenterFactory {
public:
    //unique_ptr allows to store and mange objects of any class derived from SegmenterBase without knowing the exact type at compile time
    //call appropriate constructor based on style and return pointer to created object
    static unique_ptr<SegmenterBase> create(SegmentationStyle style) {
        //checks style and creates appropriate segmenter object
        switch (style) {
            case SegmentationStyle::Threshold:
                return make_unique<ThresholdSegmenter>(
                    ConfigLoader::getParam("Threshold", "threshold"),
                    ConfigLoader::getParam("Threshold", "maxValue")
                );
            case SegmentationStyle::Canny:
                return make_unique<CannySegmenter>(
                    ConfigLoader::getParam("Canny", "lowThreshold"),
                    ConfigLoader::getParam("Canny", "highThreshold")
                );
            default:
                throw invalid_argument("Unknown segmentation style.");
        }
    }
};

int main(int argc, char* argv[]) {
    ConfigLoader::loadConfig("segmenter_config.json"); //load configuration parameters from JSON file

    const string path = "/mnt/netapp/SECURITY/users/rnguyen/segModule/card.jpg";
    const Mat img = imread(path);
    if (img.empty()) {
        cerr << "Failed to load image: " << path << '\n';
        return 1;
    }
    //get active segmentation style from config and create appropriate segmenter object using factory
    const string styleName = ConfigLoader::getActiveStyle();
    const SegmentationStyle style = parseStyle(styleName);

    const unique_ptr<SegmenterBase> segmenter = SegmenterFactory::create(style);
    //segment the image using the created segmenter object
    const Mat segmented = segmenter->segment(img);

    imshow("Original", img);
    imshow("Segmented - " + styleName, segmented);
    waitKey(0);
    return 0;
}