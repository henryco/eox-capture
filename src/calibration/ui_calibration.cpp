//
// Created by henryco on 8/20/23.
//

#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../utils/stb_image.h"
#include "../v4l2/linux_video.h"
#include "../utils/utils.h"
#include "../utils/mappers/cam_gtk_mapper.h"

void UiCalibration::prepareCamera() {

    // TEMPORAL (move to CLI later)
    const std::map<uint, uint> devices = {
            // ID, INDEX
            {1, 4},
            {2, 2}
    };
    const std::string codec = "MJPG";
    const int width = 640;
    const int height = 480;
    const int fps = 30;
    const bool homogeneous = true;
    const bool fast = false;
    const int api = cv::CAP_V4L2;
    const int buffer = 2;
    // TEMPORAL (move to CLI later)


    {
        // Init camera
        std::vector<sex::xocv::CameraProp> props;
        props.reserve(devices.size());
        for (const auto &[id, index]: devices) {
            props.emplace_back(id, index, width, height, codec, fps, buffer);
        }

        camera.setHomogeneous(homogeneous);
        camera.setFast(fast);
        camera.setApi(api);
        camera.open(props);

        const auto controls = camera.getControls();
        for (const auto &control: controls) {

            const auto indexes = sex::mappers::cam_gtk::index(
                    props,control.id, homogeneous);

            auto cam_params = std::make_unique<sex::xgtk::GtkCamParams>();
            cam_params->setProperties(sex::mappers::cam_gtk::map(control.controls));
            cam_params->onUpdate(updateCamera(indexes));
            cam_params->onReset(resetCamera(indexes));
            cam_params->onSave(saveCamera(indexes));

            configStack.add(*cam_params, " Camera " + std::to_string(control.id));
            keep(std::move(cam_params));
        }
    }

    {
        // Init oGL canvas
        glImage.init((int) devices.size(), width, height, GL_BGR);
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
