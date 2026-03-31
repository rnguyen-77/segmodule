
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <memory>

using namespace cv;
using namespace std;

enum class SegmentationStyle {
    Threshold,
    Canny
    //insert additional styles here
};

class ISegmenter {
public:
    virtual ~ISegmenter() = default;
    virtual Mat segment(const Mat& input) const = 0;
    virtual string name() const = 0;
    //virtual methods on basis of segmentation styles 
};

class ThresholdSegmenter : public ISegmenter {
public:
    //constructor with default(necessary) parameters for thresholding
    //ThresholdSegmenter : initialize object
    //Mat segment : segment function
    //string name : name function
private:
    //stored values
    //untouchable to user
    double threshold_;
    double maxValue_;
    Mat toGray(const Mat& input) const {
      //helper function
      //calls segment function
    }
};

class CannySegmenter : public ISegmenter {
public:
    //same logic as ThresholdSegmenter but for Canny edge detection
    /*
    CannySegmenter
    Mat segment
    string name
    */

private:
    double lowThreshold_;
    double highThreshold_;
    Mat toGray(const Mat& input) const {
    }
    //if segmenters share some helper functions consider creating a base class with common functionality to avoid code duplication and promote reusability
    //protected access specifier allows derived classes to use helper functions while keeping them hidden from users of segmenter objects
};

//factory class to create segmenter objects based on user input
//does not specify the exact class because that is unknown until time of creation
class SegmenterFactory {
public:
    //unique_ptr allows to store and mange objects of any class derived from ISegmenter without knowing the exact type at compile time
    //call appropriate constructor based on style and return pointer to created object
    static unique_ptr<ISegmenter> create(SegmentationStyle style) {
        //checks style and creates appropriate segmenter object
        switch (style) {
            case SegmentationStyle::Threshold:
                return make_unique<ThresholdSegmenter>(127.0, 255.0);
            case SegmentationStyle::Canny:
                return make_unique<CannySegmenter>(50.0, 150.0);
            default:
                throw invalid_argument("Unknown segmentation style.");
        }
    }
    //consider factory accepting parameters for segmenter construction to allow user customization of segmentation parameters

    //for future modification of factory styles consider registry pattern to allow dynamic registration of new segmenter types without modifying factory code
};

int main(int argc, char* argv[]) {

    const string path = "/mnt/netapp/SECURITY/users/rnguyen/segModule/card.jpg";
    const Mat img = imread(path);
    if (img.empty()) {
        cerr << "Failed to load image: " << path << '\n';
        return 1;
    }
    //user input for segmentation style
    const unique_ptr<ISegmenter> segmenter = SegmenterFactory::create(style);
    //segment the image using the created segmenter object
    const Mat segmented = segmenter->segment(img);

    imshow("Original", img);
    imshow("Segmented - " + segmenter->name(), segmented);
    waitKey(0);
    return 0;
}