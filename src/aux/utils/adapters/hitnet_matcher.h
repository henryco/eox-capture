//
// Created by henryco on 1/19/24.
//

#ifndef STEREOX_HITNET_MATCHER_H
#define STEREOX_HITNET_MATCHER_H

#include <opencv2/calib3d.hpp>
#include "../../dnn/hitnet_disparity.h"

namespace eox::adapt {

    class HitNetMatcher : public cv::StereoMatcher {

    private:
        eox::dnn::HitNetDisparity instance;

    public:
        HitNetMatcher &setPrecision(eox::dnn::Precision precision);

        HitNetMatcher &setWidth(size_t width);

        HitNetMatcher &setHeight(size_t height);

        [[nodiscard]] size_t getWidth() const;

        [[nodiscard]] size_t getHeight() const;

        [[nodiscard]] eox::dnn::Precision getPrecision() const;

        static cv::Ptr<HitNetMatcher> create();

        static cv::Ptr<HitNetMatcher> create(size_t width, size_t height);

        void compute(cv::InputArray &left, cv::InputArray &right, cv::OutputArray &disparity) override;

        [[nodiscard]] int getMinDisparity() const override;

        void setMinDisparity(int minDisparity) override;

        [[nodiscard]] int getNumDisparities() const override;

        void setNumDisparities(int numDisparities) override;

        [[nodiscard]] int getBlockSize() const override;

        void setBlockSize(int blockSize) override;

        [[nodiscard]] int getSpeckleWindowSize() const override;

        void setSpeckleWindowSize(int speckleWindowSize) override;

        [[nodiscard]] int getSpeckleRange() const override;

        void setSpeckleRange(int speckleRange) override;

        [[nodiscard]] int getDisp12MaxDiff() const override;

        void setDisp12MaxDiff(int disp12MaxDiff) override;
    };

} // eox

#endif //STEREOX_HITNET_MATCHER_H
