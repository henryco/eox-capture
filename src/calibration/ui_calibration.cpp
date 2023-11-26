//
// Created by henryco on 8/20/23.
//

#include <glibmm/main.h>
#include <iostream>
#include <gtkmm/textview.h>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../utils/stb_image.h"
#include "../v4l2/linux_video.h"

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
    const bool separate = false;
    const int api = cv::CAP_V4L2;
    const int buffer = 2;
    // TEMPORAL (move to CLI later)


    {
        // Camera controls and so on

        auto stack = std::make_unique<Gtk::Stack>();
        auto switcher = std::make_unique<Gtk::StackSwitcher>();
        switcher->set_stack(*stack);


        if (api == cv::CAP_V4L2) {
            // Camera controls for V4L2 API (video for linux)

            const size_t rounds = separate ? index.size() : 1;
            for (size_t i = 0; i < rounds; i++) {
                auto v4_props = sex::v4l2::get_camera_props(index[i]);
                std::vector<sex::xgtk::GtkCamProp> parameters;
                for (const auto &p: v4_props) {
                    if (p.type == 6)
                        continue;
                    parameters.emplace_back(
                            p.id,
                            p.type,
                            std::string(reinterpret_cast<const char *>(p.name), 32),
                            p.minimum,
                            p.maximum,
                            p.step,
                            p.default_value,
                            p.value
                    );
                }
                auto cam_params = std::make_unique<sex::xgtk::GtkCamParams>();
                cam_params->onUpdate(updateCamera(separate ? std::vector<uint>{index[i]} : index));
                cam_params->setProperties(parameters);

                const auto postfix = separate ? std::to_string(index[i]) : "";
                stack->add(*cam_params, "cam_prop" + postfix, " Camera " + postfix);

                widgets.push_back(std::move(cam_params));

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
            }
        }

        layout_v.pack_start(*switcher, Gtk::PACK_SHRINK);
        layout_v.pack_start(*stack);

        widgets.push_back(std::move(stack));
        widgets.push_back(std::move(switcher));
    }


    {
        // OPENCV camera config

        std::vector<sex::xocv::CameraProp> props;
        props.reserve(index.size());
        for (const auto i: index) {
            props.emplace_back(i, width, height, codec, fps, buffer, api);
        }

        int min_fps = 0;
        int min_w = 0;
        int min_h = 0;
        for (const auto &prop: props) {
            if (min_fps == 0 || prop.fps < min_fps)
                min_fps = prop.fps;
            if (min_w == 0 || prop.width < min_w)
                min_w = prop.width;
            if (min_h == 0 || prop.height < min_h)
                min_h = prop.height;
        }

        glImage.init((int) index.size(), min_w, min_h, GL_BGR);
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
    layout_h.pack_start(layout_v, Gtk::PACK_SHRINK);

    dispatcher.connect(sigc::mem_fun(*this, &UiCalibration::on_dispatcher_signal));
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

    glImage.setFrames(std::move(captured));
    dispatcher.emit();
}

void UiCalibration::on_dispatcher_signal() {
    set_title("StereoX++ calibration [ " + std::to_string((int) FPS) + " FPS ]");
    glImage.update();
}

UiCalibration::~UiCalibration() {
    log->debug("terminate calibration");

    deltaLoop.stop();
    camera.release();

    log->debug("terminated");
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


