//
// Created by henryco on 11/15/23.
//

#include "stereo_camera.h"

#include <iostream>
#include <utility>

void initFromParams(cv::VideoCapture& capture, const CameraProp& prop) {
    std::vector<int> params;
    // todo
    params.assign({
        cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc(
                prop.codec[0],
                prop.codec[1],
                prop.codec[2],
                prop.codec[3]),
        cv::CAP_PROP_FRAME_WIDTH, prop.width,
        cv::CAP_PROP_FRAME_HEIGHT, prop.height,
        cv::CAP_PROP_FPS, prop.fps
    });
    capture.open(prop.index, prop.api, params);
}

StereoCamera::StereoCamera(const std::vector<CameraProp>& props) {
    open(props);
}

StereoCamera::StereoCamera(StereoCamera&& other) noexcept
: captures(std::move(other.captures)), properties(std::move(other.properties)) {}


std::vector<cv::Mat> StereoCamera::capture() {
    std::vector<cv::Mat> frames;

    if (captures.empty()) {
        std::cerr << "StereoCamera is not initialized" << std::endl;
    }

    // TODO REWORK AND THREAD POOL

    for (auto& capture : captures) {
        std::cout << "grab" << std::endl;
        capture->grab();
    }

    for (auto& capture : captures) {
        std::cout << "capture" << std::endl;

        cv::Mat frame;
        capture->retrieve(frame);
        if (frame.empty())
            throw std::runtime_error("Frame is empty");
        frames.push_back(frame);
    }
    return frames;
}

void StereoCamera::open() {
    for (const auto& property : properties) {
        std::cout << "open [" << property.index << "]" << std::endl;
        auto capture = std::make_unique<cv::VideoCapture>();
        initFromParams(*capture, property);
        if (!capture->isOpened())
            throw std::runtime_error("Failed to open camera: " + std::to_string(property.index));
        captures.push_back(std::move(capture));
    }
    std::cout << "opened: " <<  captures.size() << std::endl;
}

void StereoCamera::open(std::vector<CameraProp> props) {
    properties = std::move(props);
    open();
}

void StereoCamera::release() {
    for (auto& capture : captures)
        capture->release();
    captures.clear();
}

StereoCamera::~StereoCamera() {
    release();
}

const std::vector<CameraProp>& StereoCamera::getProperties() const {
    return properties;
}
