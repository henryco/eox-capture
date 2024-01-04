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

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"

namespace eox::dnn {

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
        void init();

    public:
        BlazePose();

        ~BlazePose();

        void forward(cv::InputArray &frame);

        void inference(const float *frame);
    };

} // eox

#endif //STEREOX_BLAZE_POSE_H
