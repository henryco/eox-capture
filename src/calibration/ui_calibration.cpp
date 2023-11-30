//
// Created by henryco on 8/20/23.
//

#include <gtkmm/filechooserdialog.h>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../libs/stb/stb_image.h"
#include "../v4l2/linux_video.h"
#include "../utils/utils.h"
#include "../utils/mappers/cam_gtk_mapper.h"

void UiCalibration::init(const sex::data::basic_config &configuration) {
    const auto &props = configuration.camera;

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

            configStack.add(*cam_params, " Camera " + std::to_string(control.id));
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
        add(layout_h);
        layout_h.pack_start(glImage, Gtk::PACK_SHRINK);
        layout_h.pack_start(configStack, Gtk::PACK_SHRINK);
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
        log->debug("saveCamera");
        // TODO SERIALIZATION

        Gtk::FileChooserDialog dialog("Please select a file to save", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_transient_for(*this);

        dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Save", Gtk::RESPONSE_OK);

        const int result = dialog.run();
        switch (result) {
            case Gtk::RESPONSE_OK: {
                auto const file_name = dialog.get_filename();
                log->info("selected file: {}", file_name);
                break;
            }
            default:
                log->debug("nothing selected");
                break;
        }
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
