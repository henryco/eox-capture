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

namespace eox::dnn {

    using DetectedPose = struct {

        /**
         * RoI for face
         */
        RoI face;

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

    class PoseDetector {
        static inline const auto log =
                spdlog::stdout_color_mt("pose_detector");

    private:
        static inline const std::string file = "./../models/blazepose/blazepose_detection_float32.tflite";
        static inline const size_t in_resolution = 224;
        static const std::vector<std::string> outputs;
        std::unique_ptr<tflite::FlatBufferModel> model;
        std::unique_ptr<tflite::Interpreter> interpreter;
        TfLiteDelegate *gpu_delegate = nullptr;
        bool initialized = false;

        std::vector<std::array<float, 4>> anchors_vec;
        float threshold = 0.5;

        int view_w = 0;
        int view_h = 0;

        eox::dnn::PoseRoi roiPredictor;

    protected:
        std::vector<eox::dnn::DetectedPose> process();

    public:
        PoseDetector();

        ~PoseDetector();

        void init();

        std::vector<eox::dnn::DetectedPose> inference(cv::InputArray &frame);

        std::vector<eox::dnn::DetectedPose> inference(const float *frame, int w, int h);

        void setThreshold(float threshold);

        [[nodiscard]] float getThreshold() const;
    };

} // eox

#endif //STEREOX_POSE_DETECTOR_H
