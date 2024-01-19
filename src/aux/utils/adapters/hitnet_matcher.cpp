//
// Created by henryco on 1/19/24.
//

#include "hitnet_matcher.h"

namespace eox::adapt {

    HitNetMatcher &HitNetMatcher::setWidth(size_t width) {
        instance.setWidth(width);
        return *this;
    }

    HitNetMatcher &HitNetMatcher::setHeight(size_t height) {
        instance.setHeight(height);
        return *this;
    }

    HitNetMatcher &HitNetMatcher::setPrecision(eox::dnn::Precision precision) {
        instance.setPrecision(precision);
        return *this;
    }

    size_t HitNetMatcher::getWidth() const {
        return instance.getWidth();
    }

    size_t HitNetMatcher::getHeight() const {
        return instance.getHeight();
    }

    eox::dnn::Precision HitNetMatcher::getPrecision() const {
        return instance.getPrecision();
    }

    cv::Ptr<HitNetMatcher> HitNetMatcher::create() {
        return create(640, 480);
    }

    cv::Ptr<HitNetMatcher> HitNetMatcher::create(size_t width, size_t height) {
        auto ptr = cv::makePtr<HitNetMatcher>();
        ptr->setWidth(width);
        ptr->setHeight(height);
        return ptr;
    }

    void HitNetMatcher::compute(cv::InputArray &left, cv::InputArray &right, cv::OutputArray &disparity) {
        auto result = instance.inference(left, right);
        disparity.assign(result.disparity);
    }

    int HitNetMatcher::getMinDisparity() const {
        return 0; // mock
    }

    void HitNetMatcher::setMinDisparity(int minDisparity) {
        // mock
    }

    int HitNetMatcher::getNumDisparities() const {
        return 0; // mock
    }

    void HitNetMatcher::setNumDisparities(int numDisparities) {
        // mock
    }

    int HitNetMatcher::getBlockSize() const {
        return 0; // mock
    }

    void HitNetMatcher::setBlockSize(int blockSize) {
        // mock
    }

    int HitNetMatcher::getSpeckleWindowSize() const {
        return 0; // mock
    }

    void HitNetMatcher::setSpeckleWindowSize(int speckleWindowSize) {
        // mock
    }

    int HitNetMatcher::getSpeckleRange() const {
        return 0; // mock
    }

    void HitNetMatcher::setSpeckleRange(int speckleRange) {
        // mock
    }

    int HitNetMatcher::getDisp12MaxDiff() const {
        return 0; // mock
    }

    void HitNetMatcher::setDisp12MaxDiff(int disp12MaxDiff) {
        // mock
    }

} // eox