//
// Created by henryco on 1/12/24.
//

#ifndef STEREOX_POSE_DETECTOR_H
#define STEREOX_POSE_DETECTOR_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <opencv2/core/mat.hpp>

#include "ssd/ssd_anchors.h"
#include "roi/pose_roi.h"
#include "dnn_common.h"
#include "dnn_runner.h"

namespace eox::dnn {

    using DetectedPose = struct {

        /**
         * RoI for face
         */
        Box face;

        /**
         * RoI for body
         */
        RoI body;

        /**
         * Key point 0 - mid hip center
         * Key point 1 - point that encodes size & rotation (for full body)
         * Key point 2 - mid shoulder center
         * Key point 3 - point that encodes size & rotation (for upper body)
         */
        Point points[4];

        /**
         * Probability [0,1]
         */
        float score;

        /**
         * from -Pi to Pi radians
         */
        float rotation;
    };

    class PoseDetector : DnnRunner<std::vector<eox::dnn::DetectedPose>> {
        static inline const auto log =
                spdlog::stdout_color_mt("pose_detector");

    private:
        static inline const size_t in_resolution = 224;

        eox::dnn::PoseRoi roiPredictor;
        std::vector<std::array<float, 4>> anchors_vec;
        float threshold = 0.5;
        int view_w = 0;
        int view_h = 0;

    protected:
        void initialize() override;

    public:
        std::string get_model_file() override;

        std::vector<DetectedPose> inference(const float *frame) override;

        std::vector<DetectedPose> inference(cv::InputArray &frame);

        void setThreshold(float threshold);

        [[nodiscard]] float getThreshold() const;
    };

} // eox

#endif //STEREOX_POSE_DETECTOR_H
