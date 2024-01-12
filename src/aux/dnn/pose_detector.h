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
#include "dnn_common.h"

namespace eox::dnn {

    class PoseDetector {
        static inline const auto log =
                spdlog::stdout_color_mt("pose_detector");

    private:
        static inline const std::string file = "./../models/blazepose_detection_float32.tflite";
        static inline const size_t in_resolution = 224;

        static const std::vector<std::string> outputs;

        std::unique_ptr<tflite::FlatBufferModel> model;
        std::unique_ptr<tflite::Interpreter> interpreter;
        TfLiteDelegate *gpu_delegate = nullptr;

        bool initialized = false;

    protected:
        std::vector<eox::dnn::DetectedRegion> process();

    public:
        PoseDetector();

        ~PoseDetector();

        void init();

        std::vector<eox::dnn::DetectedRegion> inference(cv::InputArray &frame);

        std::vector<eox::dnn::DetectedRegion> inference(const float *frame);

    };

} // eox

#endif //STEREOX_POSE_DETECTOR_H
