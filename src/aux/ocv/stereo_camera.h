//
// Created by henryco on 11/15/23.
//

#ifndef STEREO_CAMERA_H
#define STEREO_CAMERA_H
#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>
#include "../utils/tp/thread_pool.h"
#include "../commons.h"

namespace sex::xocv {

    typedef struct {
        uint id;
        uint type;
        std::string name;
        int min;
        int max;
        int step;
        int default_value;
        int value;
    } camera_control;

    typedef struct {
        uint id;
        std::vector<camera_control> controls;
    } camera_controls;

    /**
     * @class StereoCamera
     * @brief The StereoCamera class represents a stereo ocv for capturing stereo images.
     */

    class StereoCamera final {

    private:
        static inline const auto log =
                spdlog::stdout_color_mt("stereo_camera");

        std::vector<std::unique_ptr<cv::VideoCapture>> captures;
        std::vector<sex::data::camera_properties> properties;
        std::shared_ptr<sex::util::ThreadPool> executor;

        bool homogeneous = true;
        bool fast = false;
        int api = cv::CAP_V4L2;

    public:

        StereoCamera() = default;

        ~StereoCamera();

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

        void open(std::vector<sex::data::camera_properties> props);

        /**
         * @brief Get the properties of the stereo camera.
         *
         * This function retrieves the properties of the stereo camera. The properties include
         * information such as resolution, focal length, and distortion coefficients.
         *
         * @return A structure containing the properties of the stereo camera.
         */
        [[nodiscard]] const std::vector<sex::data::camera_properties>& getProperties() const;

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

        void setHomogeneous(bool homogeneous);

        void setApi(int api);

        /**
         * Restore camera settings from data from input stream
         * @param input_stream data stream with camera configuration
         */
        void restore(std::istream& input_stream);

        /**
         * Write camera configuration to output stream (for every devices)
         *
         * @param output_stream output stream to write to
         */
        void save(std::ostream &output_stream);

        /**
         * Write camera configuration to output stream
         *
         * @param output_stream output stream to write to
         * @param devices vector of device identifiers
         */
        void save(std::ostream &output_stream, const std::vector<uint> &devices);

        std::vector<camera_controls> getControls();

        void setProperties(std::vector<sex::data::camera_properties> props);

        void setThreadPool(std::shared_ptr<sex::util::ThreadPool> executor);
    };

}

#endif //STEREO_CAMERA_H
