//
// Created by henryco on 8/20/23.
//

#include <numeric>
#include <fstream>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../aux/v4l2/linux_video.h"
#include "../aux/utils/mappers/cam_gtk_mapper.h"
#include "../events/events.h"
#include "../aux/gtk/gtk_config_stack.h"
#include <gtkmm/box.h>

void UiCalibration::init(sex::data::basic_config configuration) {
    config = std::move(configuration);

    const auto &props = config.camera;
    auto layout_h = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto config_stack = std::make_unique<sex::xgtk::GtkConfigStack>();

    {
        // Init camera
        camera.setHomogeneous(props[0].homogeneous);
        camera.setFast(props[0].fast);
        camera.setApi(props[0].api);
        camera.open(props);

        const auto controls = camera.getControls();
        for (const auto &control: controls) {

            const auto indexes = sex::mappers::cam_gtk::index(
                    props, control.id, props[0].homogeneous);

            auto cam_params = std::make_unique<sex::xgtk::GtkCamParams>();
            cam_params->setProperties(sex::mappers::cam_gtk::map(control.controls));
            cam_params->onUpdate(updateCamera(indexes));
            cam_params->onReset(resetCamera(indexes));
            cam_params->onSave(saveCamera(indexes));

            config_stack->add(*cam_params, " Camera " + std::to_string(control.id));
            keep(std::move(cam_params));
        }
    }

    {
        // Init oGL canvas
        glImage.init((int) props.size(), props[0].width, props[0].height, GL_BGR);
    }

    {
        // Stable FPS worker loop
        deltaLoop.setFunc([this](float d, float l, float f) { update(d, l, f); });
        deltaLoop.setFps(0);
        deltaLoop.start();
    }

    {
        // Init Window
        layout_h->pack_start(glImage, Gtk::PACK_SHRINK);
        layout_h->pack_start(*config_stack, Gtk::PACK_SHRINK);
        add(*layout_h);
        keep(std::move(layout_h));
        keep(std::move(config_stack));
        show_all_children();
    }
}

void UiCalibration::update(float delta, float latency, float _fps) {
//    log->debug("update: {}, late: {}, fps: {}", delta, latency, _fps);
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
    return [ids = std::move(devices)](uint prop_id, int value) -> int {
        log->debug("update_property: {}, {}", prop_id, value);
        for (const auto &id: ids) {
            sex::v4l2::set_camera_prop(id, prop_id, value);
        }
        return value;
    };
}

std::function<void()> UiCalibration::saveCamera(std::vector<uint> devices) {
    return [this, ids = std::move(devices)]() {
        log->debug("save camera configuration");
        sex::events::gtk_save_camera_settings_event(ids, *this, log);
        log->debug("camera configuration done");
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
