#include "bag_adapter.hpp"

#include <cmath>     // file_io.h uses std::ceil
#include <vector>
#include <stdexcept>

#include <opencv2/imgproc.hpp>

#include "bag.h"     // coworker's bagLoader: namespace ALOG, boost::multi_array
                     // (this is the ONLY TU that includes it — see bag_adapter.hpp)

cv::Mat loadBagSliceAsMat(const std::string& path) {
    // ALOG::Bag's constructor takes a char* and reads the (gzip-compressed)
    // .mi.gz file into its `img` member (a boost::multi_array<unsigned short,3>).
    std::vector<char> name(path.begin(), path.end());
    name.push_back('\0');
    ALOG::Bag bag(name.data());   // throws std::runtime_error on a bad/missing file

    const auto& vol = bag.img;    // [z][y][x], unsigned short (16-bit intensities)
    const std::size_t nz = vol.shape()[0];
    const std::size_t ny = vol.shape()[1];
    const std::size_t nx = vol.shape()[2];
    if (nz == 0 || ny == 0 || nx == 0) {
        throw std::runtime_error("loadBagSliceAsMat: bag volume is empty");
    }

    // Take the middle slice along z as a representative 2-D image. boost::multi_array
    // uses C (row-major) storage by default, so slice z is the contiguous block of
    // ny*nx elements starting at z*ny*nx.
    const std::size_t z = nz / 2;
    const unsigned short* base = vol.data() + z * ny * nx;

    // Wrap that block as a 16-bit Mat (no copy), then normalize into an 8-bit Mat.
    // normalize() allocates its own buffer, so the result stays valid after `bag`
    // (and its volume) is destroyed when this function returns.
    cv::Mat slice16(static_cast<int>(ny), static_cast<int>(nx), CV_16U,
                    const_cast<unsigned short*>(base));
    cv::Mat slice8;
    cv::normalize(slice16, slice8, 0, 255, cv::NORM_MINMAX, CV_8U);
    return slice8;
}
