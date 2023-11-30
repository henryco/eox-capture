//
// Created by henryco on 11/30/23.
//

#include <numeric>
#include <string>
#include <gtkmm/filefilter.h>
#include <gtkmm/filechooserdialog.h>
#include <fstream>
#include "events.h"
#include "../aux/v4l2/linux_video.h"

namespace sex::events {

    void gtk_save_camera_settings_event(
            const std::vector<uint> &devices,
            Gtk::Window &window,
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

                    map.emplace(index, controls);
                }

                std::ofstream file_stream(file_name, std::ios::out);
                if (!file_stream) {
                    log->error("File stream opening error");
                    // maybe throw some exception or show some modal?
                    return;
                }

                for (const auto &[index, controls]: map) {
                    log->debug("writing camera [{}] configuration to file: {}", index, file_name);
                    sex::v4l2::write_control(file_stream, index, controls);
                }

                file_stream.close();
                break;
            }
            default:
                log->debug("nothing selected");
                break;
        }
    }

} // events