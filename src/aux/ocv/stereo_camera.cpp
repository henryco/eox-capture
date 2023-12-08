//
// Created by henryco on 11/15/23.
//

#include "./../v4l2/linux_video.h"
#include "stereo_camera.h"

#include <utility>
#include <iostream>

namespace sex::xocv {

    int fourCC(const char *name) {
        return cv::VideoWriter::fourcc(
                name[0],
                name[1],
                name[2],
                name[3]);
    }

    void init_from_params(cv::VideoCapture &capture, const sex::data::camera_properties &prop, int api) {
        std::vector<int> params;
        params.assign({
                              cv::CAP_PROP_FOURCC, fourCC(prop.codec),
                              cv::CAP_PROP_FRAME_WIDTH, prop.width,
                              cv::CAP_PROP_FRAME_HEIGHT, prop.height,
                              cv::CAP_PROP_FPS, prop.fps,
                              cv::CAP_PROP_BUFFERSIZE, prop.buffer
                      });
        capture.open((int) prop.index, api, params);
    }

    StereoCamera::StereoCamera(StereoCamera &&other) noexcept:
            captures(std::move(other.captures)),
            properties(std::move(other.properties)) {
    }

    std::vector<camera_controls> StereoCamera::getControls() {
        if (api == cv::CAP_V4L2) {
            std::vector<camera_controls> vec;
            for (const auto &prop: properties) {

                const auto control = sex::v4l2::get_camera_props(prop.index);
                std::vector<camera_control> controls;

                for (const auto &ctr: control) {
                    if (ctr.type == 6)
                        continue;
                    controls.push_back({
                                               .id = ctr.id,
                                               .type = ctr.type,
                                               .name = std::string(reinterpret_cast<const char *>(ctr.name), 32),
                                               .min = ctr.minimum,
                                               .max = ctr.maximum,
                                               .step = ctr.step,
                                               .default_value = ctr.default_value,
                                               .value = ctr.value
                                       });
                }

                if (homogeneous) {
                    camera_controls data = {.id = 0, .controls = std::move(controls)};
                    vec.emplace_back(data);
                    return vec;
                }

                camera_controls data = {.id = prop.id, .controls = std::move(controls)};
                vec.emplace_back(data);
            }
            return vec;
        } else {
            // TODO windows support
            return {};
        }
    }

    std::map<uint, cv::Mat> StereoCamera::captureWithId() {
        std::map<uint, cv::Mat> map;
        auto captured = capture();
        for (int i = 0; i < properties.size(); i++) {
            map.emplace(
                    properties.at(i).id,
                    captured.at(i)
            );
        }
        return map;
    }

    std::vector<cv::Mat> StereoCamera::capture() {
        std::vector<cv::Mat> frames;

        if (captures.empty()) {
            log->warn("StereoCamera is not initialized");
        }

        std::vector<cv::VideoCapture> cameras;
        std::vector<int> ready;
        cameras.reserve(captures.size());
        ready.reserve(captures.size());
        for (auto &cam: captures) {
            cameras.push_back(*cam);
        }


        if (fast) {
            // faster because it calls for buffer often, but less synchronized method of grabbing frames
            if (!cv::VideoCapture::waitAny(cameras, ready))
                return {};
        } else {
            // slower, but more precise (synchronized) method of grabbing frames
            for (auto &item: cameras) {
                if (!item.grab())
                    return {};
            }
        }

        std::vector<std::future<cv::Mat>> results;
        results.reserve(captures.size());
        for (auto &capture: captures) {
            results.push_back(executor->execute<cv::Mat>([&capture]() mutable -> cv::Mat {
                cv::Mat frame;
                capture->retrieve(frame);
                return frame;
            }));
        }

        for (auto &future: results) {
            auto frame = future.get();
            if (frame.empty()) {
                log->warn("empty frame");
                return frames;
            }
            frames.push_back(std::move(frame));
        }

        return frames;
    }

    void StereoCamera::open() {
        for (const auto &property: properties) {

            log->debug("open [{}]", property.index);

            auto capture = std::make_unique<cv::VideoCapture>();

            init_from_params(*capture, property, api);

            if (!capture->isOpened())
                throw std::runtime_error("Failed to open camera: " + std::to_string(property.index));
            captures.push_back(std::move(capture));
        }

        if (homogeneous) {
            // ONLY FOR V4L2
            if (api == cv::CAP_V4L2) {

                // get first device
                const auto &first = properties[0];

                // read V4L2 parameters for first device
                auto v4_props = sex::v4l2::get_camera_props(first.index);

                // initialize data structure with config
                std::vector<sex::v4l2::V4L2_Control> v4_controls;
                for (const auto &prop: v4_props) {
                    if (prop.type == 6)
                        continue;
                    // add only modifiable parameters
                    v4_controls.push_back(sex::v4l2::V4L2_Control{.id = prop.id, .value = prop.value});
                }

                // set parameters for all devices
                for (int i = 1; i < properties.size(); i++) {
                    sex::v4l2::set_camera_prop(properties[i].index, v4_controls);
                }
            } else {
                // TODO: windows support
            }
        }

        if (!executor) {
            // there is no assigned executors
            executor = std::make_shared<sex::util::ThreadPool>();
            executor->start(properties.size());
        }

        log->debug("opened devices total: {}", captures.size());
    }

    void StereoCamera::open(std::vector<sex::data::camera_properties> props) {
        properties = std::move(props);
        open();
    }

    void StereoCamera::release() {
        log->debug("release");

        int i = 0;
        for (auto &capture: captures) {
            log->debug("release camera [{}]", i);

            capture->release();

            log->debug("camera released [{}]", i++);
        }
        captures.clear();
        log->debug("released");
    }

    void StereoCamera::restore(std::istream &input_stream) {
        if (input_stream.peek() == EOF)
            return;

        int32_t backend_header[1];
        input_stream.read(reinterpret_cast<char *>(backend_header), sizeof(backend_header));

        const int backend = backend_header[0];
        if (backend != api) {
            log->warn("mismatch backend for config file");
            return;
        }

        if (backend == cv::CAP_V4L2) {

            const auto controls = sex::v4l2::read_controls(input_stream);
            for (const auto &[device_id, vec]: controls) {
                log->debug("checking controls for device id: {}", device_id);

                for (const auto &prop: properties) {
                    if (device_id != prop.id)
                        // no desired device under such id, skip
                        continue;

                    log->debug("setting configuration for device: {}, index: {}", device_id, prop.index);

                    sex::v4l2::set_camera_prop(prop.index, vec);

                    log->debug("configuration set: {}[{}]", device_id, prop.index);
                    break;
                }
            }

        } else {
            // TODO windows support
        }
    }

    void StereoCamera::save(std::ostream &output_stream) {
        std::vector<uint> devices;
        devices.reserve(properties.size());
        for (const auto &prop: properties) {
            devices.push_back(prop.index);
        }
        save(output_stream, devices);
    }

    void StereoCamera::save(std::ostream &output_stream, const std::vector<uint> &devices) {
        if (api == cv::CAP_V4L2) {

            // iterative over devices
            // map <device_id, controls>
            std::map<uint, std::vector<sex::v4l2::V4L2_Control>> map;
            for (const auto &index: devices) {

                // iterating over controls for given device
                const auto _properties = sex::v4l2::get_camera_props(index);
                std::vector<sex::v4l2::V4L2_Control> controls;
                for (const auto &prop: _properties) {
                    // skipping non modifiable controls
                    if (prop.type == 6)
                        continue;
                    // remapping control
                    controls.push_back({.id = prop.id, .value = prop.value});
                }

                for (const auto &p: properties) {
                    if (p.index != index)
                        // this is definitely not our device
                        continue;

                    // id of device under corresponding index
                    map.emplace(p.id, controls);
                    break;
                }
            }

            if (!output_stream) {
                log->error("File stream opening error");
                // maybe throw some exception or show some modal?
                return;
            }

            // write backend type header first
            int32_t backend_header[1] = {(int32_t) cv::CAP_V4L2};
            output_stream.write(reinterpret_cast<const char *>(backend_header), sizeof(backend_header));

            // write aggregated config data
            for (const auto &[device_id, controls]: map) {
                log->debug("writing camera [{}] configuration", device_id);
                sex::v4l2::write_control(output_stream, device_id, controls);
            }

        } else {
            // TODO WINDOWS support?
        }
    }

    StereoCamera::~StereoCamera() {
        release();
    }

    const std::vector<sex::data::camera_properties> &StereoCamera::getProperties() const {
        return properties;
    }

    void StereoCamera::setFast(bool _fast) {
        this->fast = _fast;
    }

    void StereoCamera::setHomogeneous(bool _homogeneous) {
        this->homogeneous = _homogeneous;
    }

    void StereoCamera::setApi(int _api) {
        this->api = _api;
    }

    void StereoCamera::setProperties(std::vector<sex::data::camera_properties> props) {
        this->properties = std::move(props);
    }

    void StereoCamera::setThreadPool(std::shared_ptr<sex::util::ThreadPool> _executor) {
        this->executor = std::move(_executor);
    }

}
