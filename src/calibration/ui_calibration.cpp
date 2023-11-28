//
// Created by henryco on 8/20/23.
//

#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../utils/stb_image.h"
#include "../v4l2/linux_video.h"
#include "../utils/utils.h"

void UiCalibration::prepareCamera() {

    // TEMPORAL (move to CLI later)
    const std::vector<uint> index = {
            2,
            4
    };
    const std::string codec = "MJPG";
    const int width = 640;
    const int height = 480;
    const int fps = 30;
    const bool homogeneous = true;
    const int api = cv::CAP_V4L2;
    const int buffer = 2;
    // TEMPORAL (move to CLI later)


    {
        // Camera controls and so on

        if (api == cv::CAP_V4L2) {
            // Camera controls for V4L2 API (video for linux)

            // TODO DESERIALIZATION

            const size_t rounds = !homogeneous ? index.size() : 1;
            for (size_t i = 0; i < rounds; i++) {

                auto v4_props = sex::v4l2::get_camera_props(index[i]);

                if (rounds == 1) {
                    // multi-camera config homogenisation

                    std::vector<sex::v4l2::V4L2_Control> v4_controls;
                    for (const auto &prop: v4_props) {
                        if (prop.type == 6)
                            continue;
                        v4_controls.push_back(sex::v4l2::V4L2_Control{.id = prop.id, .value = prop.value});
                    }

                    for (size_t j = 1; j < index.size(); j++) {
                        sex::v4l2::set_camera_prop(index[j], v4_controls);
                    }
                }

                std::vector<sex::xgtk::GtkCamProp> parameters;
                for (const auto &p: v4_props) {
                    if (p.type == 6)
                        continue;

                    parameters.emplace_back(p.id, p.type, sex::utils::to_string(p.name, 32),
                                            p.minimum, p.maximum, p.step, p.default_value, p.value);
                }

                auto cam_params = std::make_unique<sex::xgtk::GtkCamParams>();
                cam_params->onUpdate(updateCamera(!homogeneous ? std::vector<uint>{index[i]} : index));
                cam_params->onReset(resetCamera(!homogeneous ? std::vector<uint>{index[i]} : index));
                cam_params->onSave(saveCamera(!homogeneous ? std::vector<uint>{index[i]} : index));
                cam_params->setProperties(parameters);

                configStack.add(*cam_params, " Camera " + (!homogeneous ? std::to_string(index[i]) : ""));
                keep(std::move(cam_params));

            }
        } else if (api == cv::CAP_DSHOW) {
            // DirectShow windows
            // TODO windows support
        }
    }


    {
        // OPENCV camera config

        std::vector<sex::xocv::CameraProp> props;
        props.reserve(index.size());
        for (const auto i: index) {
            props.emplace_back(i, width, height, codec, fps, buffer, api);
        }

        glImage.init((int) index.size(), width, height, GL_BGR);
        camera.open(props);
    }


    {
        // Stable FPS worker loop

        deltaLoop.setFunc([this](float d, float l, float f) { update(d, l, f); });
        deltaLoop.setFps(0);
        deltaLoop.start();
    }

}

void UiCalibration::init() {
    prepareCamera();
    add(layout_h);
    layout_h.pack_start(glImage, Gtk::PACK_SHRINK);
    layout_h.pack_start(configStack, Gtk::PACK_SHRINK);
    show_all_children();
}

void UiCalibration::update(float delta, float latency, float _fps) {
    log->debug("update: {}, late: {}, fps: {}", delta, latency, _fps);
    this->FPS = _fps;

    auto captured = camera.capture();
    if (captured.empty()) {
        log->debug("skip");
        return;
    }

    // LOGIC HERE

    glImage.setFrames(captured);
    refresh();
}

void UiCalibration::onRefresh() {
    set_title("StereoX++ calibration [ " + std::to_string((int) FPS) + " FPS ]");
    glImage.update();
}

std::function<int(uint, int)> UiCalibration::updateCamera(std::vector<uint> devices) {
    return [ids = std::move(devices), this](uint prop_id, int value) -> int {
        log->debug("update_property: {}, {}", prop_id, value);
        for (const auto &id: ids) {
            sex::v4l2::set_camera_prop(id, prop_id, value);
        }
        return value;
    };
}

std::function<void()> UiCalibration::saveCamera(std::vector<uint> devices) {
    return [ids = std::move(devices)]() {
        log->debug("saveCamera");
        // TODO SERIALIZATION
    };
}

std::function<void()> UiCalibration::resetCamera(std::vector<uint> devices) {
    return [ids = std::move(devices)]() {
        log->debug("resetCamera");
        for (const auto &id: ids) {
            sex::v4l2::reset_defaults(id);
        }
    };
}

UiCalibration::~UiCalibration() {
    log->debug("terminate calibration");

    deltaLoop.stop();
    camera.release();

    log->debug("terminated");
}
