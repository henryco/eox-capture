//
// Created by henryco on 1/10/24.
//

#ifndef STEREOX_POSE_PIPELINE_H
#define STEREOX_POSE_PIPELINE_H

#include <vector>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../aux/sig/velocity_filter.h"
#include "../aux/dnn/blaze_pose.h"
#include "../aux/dnn/roi/pose_roi.h"

namespace eox {

    class PosePipeline {

        static inline const auto log =
                spdlog::stdout_color_mt("pose_pipeline");

    private:
        std::vector<eox::sig::VelocityFilter> filters;
        eox::dnn::PoseRoi roiPredictor;
        eox::dnn::BlazePose pose;

        bool prediction = false;
        eox::dnn::RoI roi;

        bool initialized = false;
        float threshold = 0.5;


    public:
        void init();

        int pass(const cv::Mat &frame);

        int pass(const cv::Mat &frame, cv::Mat &segmented);

        int pass(const cv::Mat &frame, cv::Mat &segmented, cv::Mat &aux);

        [[nodiscard]] float getThreshold() const;

        void setThreshold(float threshold);

    protected:
        int inference(const cv::Mat &frame, cv::Mat &segmented, cv::Mat *aux);

        std::chrono::nanoseconds timestamp();

        void drawJoints(const eox::dnn::Landmark landmarks[39], cv::Mat &output) const;

        void drawLandmarks(const eox::dnn::Landmark landmarks[39], cv::Mat &output) const;

        void drawRoi(cv::Mat &output) const;
    };

} // eox

#endif //STEREOX_POSE_PIPELINE_H
