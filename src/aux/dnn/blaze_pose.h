//
// Created by henryco on 12/29/23.
//

#ifndef STEREOX_BLAZE_POSE_H
#define STEREOX_BLAZE_POSE_H

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>

#include "dnn_common.h"

namespace eox::dnn {

    class BlazePose {
        static inline const auto log =
                spdlog::stdout_color_mt("blaze_pose");

    private:
        static inline const std::string file = "./../models/blazepose_heavy_float32.tflite";
        static inline const size_t in_resolution = 256;

        static const std::vector<std::string> outputs;

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

        /**
         * @param frame BGR image (ie. cv::Mat of CV_8UC3)
         */
        PoseOutput inference(cv::InputArray &frame);

        /**
         * @param frame pointer to 256x256 row-oriented 1D array representation of 256x256x3 RGB image
         */
        PoseOutput inference(const float *frame);
    };

} // eox

#endif //STEREOX_BLAZE_POSE_H
