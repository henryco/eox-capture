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

namespace eox::dnn {

    class BlazePose {
        static inline const auto log =
                spdlog::stdout_color_mt("blaze_pose");

    private:
        static const std::vector<std::string> layer_names;
        static const std::string file;

        cv::dnn::Net net;

    public:
        BlazePose();

        void forward(const cv::UMat &frame);
    };

} // eox

#endif //STEREOX_BLAZE_POSE_H
