//
// Created by henryco on 12/29/23.
//

#ifndef STEREOX_BLAZE_POSE_H
#define STEREOX_BLAZE_POSE_H

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>

#include "dnn_common.h"

namespace eox::dnn {

    using PoseOutput = struct {

        /**
         * 39x5 normalized [0,1] landmarks
         */
        std::vector<eox::dnn::Landmark> landmarks_norm;

        /**
         * 1D 128x128 float32 array
         */
        std::vector<float> segmentation;

        /**
         * Probability [0,1]
         */
        float presence;
    };

    class BlazePose {
        static inline const auto log =
                spdlog::stdout_color_mt("blaze_pose");

    private:
        static const std::vector<std::string> outputs;
        static const std::string file;

        std::unique_ptr<tflite::FlatBufferModel> model;
        std::unique_ptr<tflite::Interpreter> interpreter;
        TfLiteDelegate* gpu_delegate = nullptr;

        bool initialized = false;

    protected:
        PoseOutput process();

    public:
        BlazePose();

        ~BlazePose();

        void init();

        PoseOutput inference(cv::InputArray &frame);

        PoseOutput inference(const float *frame);
    };

} // eox

#endif //STEREOX_BLAZE_POSE_H
