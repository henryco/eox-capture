//
// Created by henryco on 11/30/23.
//

#include <numeric>
#include <string>
#include <gtkmm/filefilter.h>
#include <gtkmm/filechooserdialog.h>
#include <fstream>
#include <filesystem>
#include "events.h"
#include "../aux/v4l2/linux_video.h"

namespace sex::events {

    void gtk_save_camera_settings_event(
            const std::vector<uint> &devices,
            Gtk::Window &window,
            const sex::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> log
    ) {
        const auto name = std::accumulate(
                devices.begin(), devices.end(), std::string(""),
                [](const std::string &acc, uint num) {
                    return acc + "_" + std::to_string(num);
                });

        const auto filter_text = Gtk::FileFilter::create();
        filter_text->set_name("Camera configuration files (*.xcam)");
        filter_text->add_pattern("*.xcam");

        Gtk::FileChooserDialog dialog("Please select a file to save", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_current_name("camera_" + name + ".xcam");
        dialog.set_current_folder(configuration.work_dir);
        dialog.add_filter(filter_text);
        dialog.set_transient_for(window);

        dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Save", Gtk::RESPONSE_OK);

        const int result = dialog.run();
        switch (result) {
            case Gtk::RESPONSE_OK: {
                auto const file_name = dialog.get_filename();
                log->debug("selected file_stream: {}", file_name);

                // iterative over devices
                // map <device_id, controls>
                std::map<uint, std::vector<sex::v4l2::V4L2_Control>> map;
                for (const auto &index: devices) {

                    // iterating over controls for given device
                    const auto properties = sex::v4l2::get_camera_props(index);
                    std::vector<sex::v4l2::V4L2_Control> controls;
                    for (const auto &prop: properties) {
                        // skipping non modifiable controls
                        if (prop.type == 6)
                            continue;
                        // remapping control
                        controls.push_back({.id = prop.id, .value = prop.value});
                    }

                    for (const auto &p: configuration.camera) {
                        if (p.index != index)
                            // this is definitely not our device
                            continue;

                        // id of device under corresponding index
                        map.emplace(p.id, controls);
                        break;
                    }
                }

                std::ofstream file_stream(file_name, std::ios::out);
                if (!file_stream) {
                    log->error("File stream opening error");
                    // maybe throw some exception or show some modal?
                    return;
                }

                for (const auto &[device_id, controls]: map) {
                    log->debug("writing camera [{}] configuration to file: {}", device_id, file_name);
                    sex::v4l2::write_control(file_stream, device_id, controls);
                }

                file_stream.close();
                break;
            }
            default:
                log->debug("nothing selected");
                break;
        }
    }

    void load_camera_from_paths(
            const std::vector<std::filesystem::path> &paths,
            const sex::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> log
    ) {
        log->debug("load_camera_from_paths");

        const auto &props = configuration.camera;
        for (const auto &path: paths) {
            const auto ext = path.extension().string();

            log->debug("checking path: {}", path.string());

            if (!std::filesystem::is_regular_file(path))
                continue;

            if (ext == ".xcam") {
                log->debug("loading configuration: {}", path.string());

                std::ifstream file_stream(path.string(), std::ios::in);

                const auto controls = sex::v4l2::read_controls(file_stream);
                for (const auto &[device_id, vec]: controls) {
                    log->debug("checking controls for device id: {}", device_id);

                    for (const auto &prop: props) {
                        if (device_id != prop.id)
                            // no desired device under such id, skip
                            continue;

                        log->debug("setting configuration for device: {}, index: {}", device_id, prop.index);

                        sex::v4l2::set_camera_prop(prop.index, vec);

                        log->debug("configuration set: {}, {}", device_id, prop.index);
                        break;
                    }

                }

                file_stream.close();
                continue;
            }

            log->debug("file: {}, not a camera configuration", path.string());
        }
    }


} // events