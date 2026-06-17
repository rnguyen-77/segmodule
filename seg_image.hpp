#pragma once

#include <string>

#include <opencv2/core.hpp>

// ─────────────────────────────────────────────────────────────────────────────
// SegImage — the segmenter's image type.
//2
// A lightweight cv::Mat-backed image wrapper. It fills the role the bagLoader
// module's ALOG::ALOG_Image class was *stubbed* for — that class is currently
// empty and unimplemented (no storage, no defined methods), so it can't be used
// as an image type yet. Keeping this type here (rather than in the bagLoader
// headers) means the segmenter and its unit tests don't have to depend on the
// bagLoader's headers or boost.
//
// The unit tests build a SegImage directly from synthetic cv::Mat data; the real
// program (basicseg) builds one from a loaded bag via load() below. The bridge to
// the coworker's actual bag loader lives in bag_adapter.{hpp,cpp}.
// ─────────────────────────────────────────────────────────────────────────────
class SegImage {
public:
    SegImage() = default;
    explicit SegImage(const cv::Mat& mat) : mat_(mat.clone()) {}

    bool empty() const { return mat_.empty(); }

    const cv::Mat& mat() const { return mat_; }
    void setMat(const cv::Mat& mat) { mat_ = mat.clone(); }

private:
    cv::Mat mat_;
};

// Loads a real bag (.mi.gz) through the bagLoader module and returns it as a
// SegImage. Defined in seg_image.cpp, which bridges to the bag adapter.
SegImage load(const std::string& path);
