//
// Created by henryco on 11/15/23.
//

#ifndef STEREO_CAMERA_H
#define STEREO_CAMERA_H
#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>
#include "../utils/tp/thread_pool.h"

namespace sex {

    class CameraProp {
    public:
        int index;
        int api;
        int width;
        int height;
        int fps;
        std::string codec;

        explicit CameraProp(
                const int index = 0,
                const int width = 640,
                const int height = 480,
                const std::string codec = "YUYV",
                const int fps = 30,
                const int api = cv::CAP_V4L2)
                : index(index),
                  width(width),
                  height(height),
                  fps(fps),
                  codec(codec),
                  api(api)
        {};
    };

    class StereoCamera final {

    private:
        static inline const auto log =
                spdlog::stdout_color_mt("stereo_camera");

        std::vector<std::unique_ptr<cv::VideoCapture>> captures;
        std::vector<CameraProp> properties;
        sex::ThreadPool executor;

    public:

        StereoCamera() = default;

        ~StereoCamera();

        explicit StereoCamera(const std::vector<CameraProp>&props);

        StereoCamera(StereoCamera&& other) noexcept;

        std::vector<cv::Mat> capture();

        void release();

        void open();

        void open(std::vector<CameraProp> props);

        [[nodiscard]] const std::vector<CameraProp>& getProperties() const;

    };

}

#endif //STEREO_CAMERA_H
