#pragma once

#include <string>
#include <opencv2/core.hpp>

// ─────────────────────────────────────────────────────────────────────────────
// Bridge to the bagLoader module (ltang's ALOG::Bag / FileIO loader).
//
// This header is deliberately "clean": it exposes only std::string + cv::Mat and
// does NOT pull in the coworker's bag.h (which declares `namespace ALOG`). That
// keeps it separate from our `class ALOG` adapter so the two never collide in a
// single translation unit. The implementation (bag_adapter.cpp) is the only place
// that includes bag.h and touches boost.
//
// Loads a .mi.gz bag from `path`, takes a representative 2-D slice of the 3-D
// volume, and returns it normalized to 8-bit single-channel (CV_8UC1) so the
// segmenter's threshold/Canny stages behave. Throws std::exception on failure.
// ─────────────────────────────────────────────────────────────────────────────
cv::Mat loadBagSliceAsMat(const std::string& path);
