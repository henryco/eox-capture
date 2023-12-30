//
// Created by henryco on 12/29/23.
//

#ifndef STEREOX_BLAZE_POSE_H
#define STEREOX_BLAZE_POSE_H

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace eox::dnn {

    class BlazePose {
        static inline const auto log =
                spdlog::stdout_color_mt("blaze_pose");

    private:
        static inline const std::string file = "../../../models/blazepose_model_float32.pb";
        static inline const std::vector<std::string> layer_names = {
                "Identity",
                "Identity_1",
                "Identity_4",
        };

        cv::dnn::Net net;

    public:
        BlazePose() {
            net = cv::dnn::readNetFromTensorflow(file);
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);
        }

        void forward(const cv::UMat &frame) {
            cv::UMat blob;

            cv::dnn::blobFromImage(
                    frame,
                    blob,
                    1.0,
                    cv::Size(256, 256),
                    cv::Scalar(),
                    true,
                    false,
                    CV_32F
            );

            net.setInput(blob);

            std::vector<cv::UMat> outputs;
            net.forward(outputs, layer_names);

            log->info("size: {}", outputs.size());
        }
    };

} // eox

#endif //STEREOX_BLAZE_POSE_H
