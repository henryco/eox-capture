//
// Created by henryco on 12/29/23.
//

#include "blaze_pose.h"

namespace eox::dnn {

    double sigmoid(double x) {
        return 1.0 / (1.0 + exp(-x));
    }

    const std::string BlazePose::file = "./../models/blazepose_model_float32.tflite";
    const std::vector<std::string> BlazePose::layer_names = {
            "Identity:0",   // [1, 195]           landmarks 3d
            "Identity_1:0", // [1, 1]             pose flag (score)
            "Identity_2:0", // [1, 128, 128, 1]   segmentation
            "Identity_3:0", // [1, 64, 64, 39]    heatmap
            "Identity_4:0", // [1, 117]           world 3d
    };

    BlazePose::BlazePose() {
        if (!std::filesystem::exists(file)) {
            log->error("File: " + file + " does not exists!");
            throw std::runtime_error("File: " + file + " does not exists!");
        }
        net = cv::dnn::readNetFromTFLite(file);
    }

    void BlazePose::forward(const cv::UMat &frame) {
        cv::UMat blob;

        // [1, 3, 256, 256]
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

        log->info("o[2]: {} | {}",
                  outputs[1].getMat(cv::ACCESS_READ).at<float>(0),
                  sigmoid(outputs[1].getMat(cv::ACCESS_READ).at<float>(0))
        );

    }
} // eox