//
// Created by henryco on 11/15/23.
//

#ifndef STEREO_CAMERA_H
#define STEREO_CAMERA_H
#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>

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

public:

    StereoCamera() = default;

    ~StereoCamera();

    explicit StereoCamera(const std::vector<CameraProp>&props);

    StereoCamera(StereoCamera&& other) noexcept;

    std::vector<cv::Mat> capture();

    void release();

    void open();

    void open(std::vector<CameraProp> props);

private:
    std::vector<cv::VideoCapture> captures;
    std::vector<CameraProp> properties;
};



#endif //STEREO_CAMERA_H
