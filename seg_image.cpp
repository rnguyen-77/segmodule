#include "seg_image.hpp"
#include "bag_adapter.hpp"

// Loads a real bag via the coworker's bagLoader (through the clean bag adapter)
// and wraps the resulting 2-D slice as a SegImage. This is the integration point:
// bag_adapter.hpp hides the coworker's `namespace ALOG` / boost behind a plain
// cv::Mat, so here we only ever deal with our own SegImage type.
SegImage load(const std::string& path) {
    return SegImage(loadBagSliceAsMat(path));
}
