//
// Created by henryco on 11/30/23.
//

#include <numeric>
#include <string>
#include <gtkmm/filefilter.h>
#include <gtkmm/filechooserdialog.h>
#include <fstream>
#include <filesystem>
#include "helpers.h"

namespace sex::events {

    void gtk_save_camera_settings_event(
            sex::xocv::StereoCamera &camera,
            const std::vector<uint> &devices,
            Gtk::Window &window,
            const sex::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger>& log
    ) {
        std::vector<uint> ids;
        for (const auto &index: devices) {
            for (const auto &p: configuration.camera) {
                if (p.index == index) {
                    ids.push_back(p.id);
                    break;
                }
            }
        }

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
        dialog.set_current_folder(configuration.work_dir);
        dialog.add_filter(filter_text);
        dialog.set_transient_for(window);

        dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Save", Gtk::RESPONSE_OK);

        const int result = dialog.run();
        if (result == Gtk::RESPONSE_OK) {

            auto const file_name = dialog.get_filename();
            log->debug("selected file_stream: {}", file_name);

            std::ofstream file_stream(file_name, std::ios::out);
            if (!file_stream) {
                log->error("File stream opening error");
                // maybe throw some exception or show some modal?
                return;
            }

            // saving camera parameters to stream
            camera.save(file_stream, devices);

            // closing the stream
            file_stream.close();

        } else {
            log->debug("nothing selected");
        }
    }

    void load_camera_from_paths(
            sex::xocv::StereoCamera &camera,
            const std::vector<std::filesystem::path> &paths,
            const std::shared_ptr<spdlog::logger> &log) {
        if (paths.empty()) {
            log->debug("list of config path is empty");
            return;
        }

        log->debug("load camera from paths");
        for (const auto &path: paths) {
            const auto ext = path.extension().string();

            log->debug("checking path: {}", path.string());
            if (!std::filesystem::is_regular_file(path))
                continue;

            if (ext == ".xcam") {
                log->debug("loading configuration: {}", path.string());
                std::ifstream file_stream(path.string(), std::ios::in);

                if (!file_stream) {
                    log->error("File stream opening error");
                    continue;
                }

                // restoring camera params from input stream
                camera.restore(file_stream);

                // closing the stream
                file_stream.close();
                continue;
            }

            log->debug("file: {}, not a camera configuration", path.string());
        }
    }


} // events