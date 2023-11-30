//
// Created by henryco on 8/20/23.
//

#include <gtkmm/filechooserdialog.h>
#include <numeric>
#include <fstream>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../aux/v4l2/linux_video.h"
#include "../aux/utils/mappers/cam_gtk_mapper.h"

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

        const auto name = std::accumulate(
                ids.begin(), ids.end(), std::string(""),
                [](const std::string &acc, uint num) {
                    return acc + "_" + std::to_string(num);
                });

        const auto filter_text = Gtk::FileFilter::create();
        filter_text->set_name("Camera configuration files (*.xcam)");
        filter_text->add_pattern("*.xcam");

        Gtk::FileChooserDialog dialog("Please select a file to save", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_current_name("camera_" + name + ".xcam");
        dialog.add_filter(filter_text);
        dialog.set_transient_for(*this);

        dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Save", Gtk::RESPONSE_OK);

        const int result = dialog.run();
        switch (result) {
            case Gtk::RESPONSE_OK: {
                auto const file_name = dialog.get_filename();
                log->info("selected file_stream: {}", file_name);

                // iterative over devices
                std::map<uint, std::vector<sex::v4l2::V4L2_Control>> map;
                for (const auto &index: ids) {

                    // iterating over controls for given device
                    const auto properties = sex::v4l2::get_camera_props(index);
                    std::vector<sex::v4l2::V4L2_Control> controls;
                    for (const auto& prop: properties) {
                        // skipping non modifiable controls
                        if (prop.type == 6)
                            continue;
                        // remapping control
                        controls.push_back({.id = prop.id, .value = prop.value});
                    }

                    map.emplace(index, controls);
                }

                std::ofstream file_stream(file_name, std::ios::out);
                if (!file_stream) {
                    log->error("File stream opening error");
                    // maybe throw some exception or show some modal?
                    return;
                }

                for (const auto& [index, controls] : map) {
                    log->debug("writing camera [{}] configuration to file: {}", index, file_name);
                    sex::v4l2::write_control(file_stream, index, controls);
                }

                file_stream.close();
                log->debug("camera configuration saved");
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
