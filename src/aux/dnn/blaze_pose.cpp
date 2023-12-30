//
// Created by henryco on 12/29/23.
//

#include "blaze_pose.h"

namespace eox::dnn {

    const std::string BlazePose::file = "./../models/blazepose_model_float32.pb";
    const std::vector<std::string> BlazePose::layer_names = {
            "Identity",
            "Identity_4",
            "Identity_1",
    };

    BlazePose::BlazePose() {
        if (!std::filesystem::exists(file)) {
            log->error("File: " + file + " does not exists!");
            throw std::runtime_error("File: " + file + " does not exists!");
        }
        net = cv::dnn::readNetFromTensorflow(file);
    }

    void BlazePose::forward(const cv::UMat &frame) {
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

        log->info(" ");
        log->info("size: {}", outputs.size());
        for (const auto &o: outputs) {
            log->info("dim: {}x{}", o.cols, o.rows);
        }

        log->info("o[2]: {}", outputs[2].getMat(cv::ACCESS_READ).at<float>(0));

    }
} // eox