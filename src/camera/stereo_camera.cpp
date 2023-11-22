//
// Created by henryco on 11/15/23.
//

#include "stereo_camera.h"

#include <utility>

namespace sex {

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
            log->error("StereoCamera is not initialized");
        }

        std::vector<std::future<bool>> grabs;
        grabs.reserve(captures.size());
        for (auto &capture: captures) {
            grabs.push_back(executor.execute<bool>([&capture]() mutable {
                log->debug("grab frame");
                return capture->grab();
            }));
        }

        for (auto &future: grabs) {
            const auto grabbed = future.get();
            if (!grabbed) {
                log->warn("Nothing grabbed");
                return frames;
            }
        }

        std::vector<std::future<cv::Mat>> results;
        results.reserve(captures.size());
        for (auto &capture: captures) {
            results.push_back(executor.execute<cv::Mat>([&capture]() mutable -> cv::Mat {
                log->debug("retrieve frame");
                cv::Mat frame;
                capture->retrieve(frame);
                return frame;
            }));
        }

        for (auto &future: results) {
            auto frame = future.get();
            if (frame.empty()) {
                log->warn("Frame is empty");
                return frames;
            }
            frames.push_back(std::move(frame));
        }

        return frames;
    }

    void StereoCamera::open() {
        for (const auto& property : properties) {

            log->debug("open [{}]", property.index);

            auto capture = std::make_unique<cv::VideoCapture>();
            initFromParams(*capture, property);
            if (!capture->isOpened())
                throw std::runtime_error("Failed to open camera: " + std::to_string(property.index));
            captures.push_back(std::move(capture));
        }

        executor.start(properties.size());

        log->debug("opened: {}", captures.size());
    }

    void StereoCamera::open(std::vector<CameraProp> props) {
        properties = std::move(props);
        open();
    }

    void StereoCamera::release() {
        log->debug("release");

        int i = 0;
        for (auto& capture : captures) {
            log->debug("release camera [{}]", i);

            capture->release();

            log->debug("camera released [{}]", i++);
        }
        captures.clear();
        executor.shutdown();

        log->debug("released");
    }

    StereoCamera::~StereoCamera() {
        release();
    }

    const std::vector<CameraProp>& StereoCamera::getProperties() const {
        return properties;
    }

}
