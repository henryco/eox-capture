//
// Created by henryco on 8/20/23.
//

#include <numeric>
#include <fstream>
#include "ui_calibration.h"
#include <gtkmm/box.h>

#include "../aux/v4l2/linux_video.h"
#include "../helpers/helpers.h"
#include "../aux/utils/mappers/cam_gtk_mapper.h"
#include "../aux/gtk/gtk_config_stack.h"
#include "../aux/gtk/gtk_utils.h"

void UiCalibration::init(sex::data::basic_config configuration) {
    config = std::move(configuration);

    const auto &props = config.camera;
    auto layout_h = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto config_stack = std::make_unique<sex::xgtk::GtkConfigStack>();

    std::vector<std::string> c_ids;
    c_ids.reserve(config.camera.size());
    for (const auto &c_p: config.camera) {
        c_ids.push_back(std::to_string(c_p.id));
    }

    {
        // Init oGL canvas
        glImage.init((int) props.size(), props[0].width, props[0].height, c_ids,GL_BGR);
        glImage.scale(config.scale);
    }

    {
        // init executor
        executor = std::make_shared<sex::util::ThreadPool>();
        executor->start(props.size());
    }

    {
        // Init camera
        camera.setProperties(props);
        camera.setThreadPool(executor);
        camera.setHomogeneous(props[0].homogeneous);
        camera.setFast(props[0].fast);
        camera.setApi(props[0].api);

        {
            // loading camera parameters from files

            {
                log->debug("initializing from dir implicitly");

                std::vector<std::filesystem::path> paths;
                for (const auto &entry: std::filesystem::directory_iterator(config.work_dir)) {
                    const auto path = entry.path().string();

                    bool contains = false;
                    for (const auto &c_path: config.configs) {
                        if (c_path == path) {
                            contains += 1;
                            break;
                        }
                    }

                    if (!contains)
                        paths.push_back(entry.path());
                }

                sex::helpers::load_camera_from_paths(camera, paths, log);

                auto data = sex::helpers::load_calibration_data(paths, log);
                for (const auto &package: data) {
                    auto solo = package.solo;

                    for (const auto &p: props) {
                        if (!solo.contains(p.id))
                            continue;

                        const auto cpy = solo[p.id];
                        preCalibrated[p.id] = cpy;

                        log->debug("set calibration data: {}", p.id);
                        break;
                    }
                }
            }

            {
                log->debug("initializing from files explicitly");

                std::vector<std::filesystem::path> paths;
                paths.reserve(config.configs.size());
                for (const auto &entry: config.configs)
                    paths.emplace_back(entry);

                sex::helpers::load_camera_from_paths(camera, paths, log);

                auto data = sex::helpers::load_calibration_data(paths, log);
                for (const auto &package: data) {
                    auto solo = package.solo;

                    for (const auto &p: props) {
                        if (!solo.contains(p.id))
                            continue;

                        const auto cpy = solo[p.id];
                        preCalibrated[p.id] = cpy;

                        log->debug("set calibration data: {}", p.id);
                        break;
                    }
                }
            }

            if (!preCalibrated.empty() && preCalibrated.size() != props.size()) {
                log->warn("missing calibration data for some devices");
                preCalibrated.clear();
            }
        }

        {
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
    }

    {
        // calibration config
        auto _layout_h = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);

        auto layout_calibration = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
        layout_calibration->pack_start(*_layout_h, Gtk::PACK_SHRINK);

        progressBar.set_text("test");
        progressBar.set_show_text(true);
        layout_calibration->pack_start(progressBar, Gtk::PACK_SHRINK);

        const std::string css = R"css(
            .button-save {
                 margin-right: 5px;
                 margin-bottom: 5px;
             }
        )css";

        start.set_halign(Gtk::ALIGN_CENTER);
        start.set_label("Start");
        start.get_style_context()->add_class("button-save");
        sex::xgtk::add_style(start, css);
        start.signal_clicked().connect([this]() {
            active = !active;
            if (active) {
                log->debug("calibration start");
                stereoPackage = {};
                start.set_label("stop");
                save.set_sensitive(false);
                image_points.clear();
                cap = 0;
                timer.reset();
                progress = 1;
            } else {
                log->debug("calibration stop");
                image_points.clear();
                stereoPackage = {};
                start.set_label("start");
                timer.reset();
                progress = 0;
            }
        });

        save.set_label("Save");
        save.set_sensitive(false);
        save.get_style_context()->add_class("button-save");
        sex::xgtk::add_style(save, css);
        save.signal_clicked().connect([this]() {
            sex::helpers::save_calibration_data(stereoPackage, *this, config, log);
        });

        _layout_h->pack_start(start, Gtk::PACK_SHRINK);
        _layout_h->pack_start(save, Gtk::PACK_SHRINK);

        config_stack->add(*layout_calibration, " Calibration ");
        keep(std::move(layout_calibration));
        keep(std::move(_layout_h));
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

    {
        // start camera
        camera.open();
    }

    {
        // Stable FPS worker loop
        deltaLoop.setFunc([this](float d, float l, float f) { update(d, l, f); });
        deltaLoop.setFps(0);
        deltaLoop.start();
    }

    {
        // Init timer
        timer.set_delay(config.calibration.delay);
        timer.start();
    }
}

void UiCalibration::onRefresh() {
    set_title("StereoX++ calibration [ " + std::to_string((int) FPS) + " FPS ]");
    progressBar.set_fraction(progress);
    start.set_label(active ? "Stop" : "Start");
    progressBar.set_text(std::to_string(cap) + " / " + std::to_string(config.calibration.number));
    if (cap >= config.calibration.number) {
        save.set_sensitive(true);
    }
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
        sex::helpers::gtk_save_camera_settings(camera, ids, *this, config, log);
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
    timer.stop();

    log->debug("terminated");
}

void UiCalibration::update_ui(int remains, std::vector<cv::Mat>& _frames) {
    const auto fraction = (double) remains / timer.get_delay();
    if (std::isfinite(fraction))
        progress = fraction;
    else progress = 0;

    glImage.setFrames(_frames);
    refresh();
}
