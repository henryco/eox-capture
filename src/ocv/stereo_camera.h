//
// Created by henryco on 11/15/23.
//

#ifndef STEREO_CAMERA_H
#define STEREO_CAMERA_H
#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>
#include "../utils/tp/thread_pool.h"

namespace sex::xocv {

    /**
     * @class CameraProp
     * @brief The CameraProp class represents the properties for capturing camera.
     *
     * This class contains various properties related to camera capture, such as index, API type,
     * width, height, frames per second (FPS), buffer size, and codec used.
     */

    class CameraProp {
    public:
        uint index;
        int api;
        int width;
        int height;
        int fps;
        int buffer;
        bool homogeneous;
        std::string codec;

        /**
         * @brief The CameraProp class represents the properties for capturing camera.
         */

        explicit CameraProp(
                const uint index = 0,
                const int width = 640,
                const int height = 480,
                std::string codec = "YUYV",
                const int fps = 30,
                const int buffer = 2,
                const int api = cv::CAP_V4L2,
                const bool homogeneous = true)
                : index(index),
                  width(width),
                  height(height),
                  fps(fps),
                  codec(std::move(codec)),
                  buffer(buffer),
                  homogeneous(homogeneous),
                  api(api)
        {};
    };

    /**
     * @class StereoCamera
     * @brief The StereoCamera class represents a stereo ocv for capturing stereo images.
     */

    class StereoCamera final {

    private:
        static inline const auto log =
                spdlog::stdout_color_mt("stereo_camera");

        std::vector<std::unique_ptr<cv::VideoCapture>> captures;
        std::vector<CameraProp> properties;
        sex::util::ThreadPool executor;

        bool fast = false;

    public:

        StereoCamera() = default;

        ~StereoCamera();

        /**
         * @class StereoCamera
         * @brief Represents a stereo ocv composed of multiple individual cameras.
         *
         * The StereoCamera class provides a convenient way to manage and control a set
         * of individual cameras that capture stereo imagery. It takes a vector of
         * CameraProp objects as input during construction, where each CameraProp object
         * represents the properties of an individual ocv, such as its resolution, etc.
         */

        explicit StereoCamera(const std::vector<CameraProp>& props);

        /**
         * @brief Move constructor for StereoCamera objects.
         *        Moves the contents of another StereoCamera object into this one.
         *
         * @param other The StereoCamera object to be moved from.
         */

        StereoCamera(StereoCamera&& other) noexcept;

        /**
         * @brief Captures an image using the stereo ocv.
         *
         * This function captures an image using the stereo ocv. It retrieves the
         * frames simultaneously and returns them as a vector of cv::Mat objects.
         *
         * @return A vector of cv::Mat objects representing the frames (usually left and right).
         *
         * @note This is blocking operation
         */

        std::vector<cv::Mat> capture();

        /**
         * This function releases any resources held by the current instance.
         */

        void release();

        /**
         * @see sex::StereoCamera::open(std::vector<CameraProp> props)
         */
        void open();

        /**
         * @brief Opens the cameras based on the provided properties.
         *
         * This function opens the cameras based on the provided properties. Each camera property
         * specifies the configuration settings for a camera. The function uses a vector of CameraProp
         * objects to represent these properties. Once the cameras have been successfully opened, they
         * can be used to capture frames or perform other camera-related operations.
         *
         * @param props A vector of CameraProp objects representing the properties of the cameras to be opened.
         * @return void
         *
         * @see sex::CameraProp
         */

        void open(std::vector<CameraProp> props);

        /**
         * @brief Get the properties of the stereo camera.
         *
         * This function retrieves the properties of the stereo camera. The properties include
         * information such as resolution, focal length, and distortion coefficients.
         *
         * @return A structure containing the properties of the stereo camera.
         */
        [[nodiscard]] const std::vector<CameraProp>& getProperties() const;

        /**
         * @brief Set the fast mode for the StereoCamera.
         *
         * This function sets the fast mode for the StereoCamera. When fast mode is enabled, certain optimizations
         * may be used to improve performance, but at the cost of potentially reduced accuracy.
         *
         * @param fast A bool indicating whether fast mode should be enabled (true) or disabled (false).
         *
         * @note Fast mode is disabled by default.
         */
        void setFast(bool fast);
    };

}

#endif //STEREO_CAMERA_H
