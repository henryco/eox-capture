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

namespace eox::helpers {

    std::vector<std::filesystem::path> config_paths(const eox::data::basic_config &configuration) {
        std::vector<std::filesystem::path> paths;
        paths.reserve(configuration.configs.size());
        for (const auto &entry: configuration.configs)
            paths.emplace_back(entry);
        return paths;
    }

    std::vector<std::filesystem::path> work_paths(const eox::data::basic_config &configuration) {
        std::vector<std::filesystem::path> paths;
        for (const auto &entry: std::filesystem::directory_iterator(configuration.work_dir)) {
            const auto path = entry.path().string();

            bool contains = false;
            for (const auto &c_path: configuration.configs) {
                if (c_path == path) {
                    contains += 1;
                    break;
                }
            }

            if (!contains)
                paths.push_back(entry.path());
        }
        return paths;
    }

    void gtk_save_camera_settings(
            eox::xocv::StereoCamera &camera,
            const std::vector<uint> &devices,
            Gtk::Window &window,
            const eox::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log
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
            log->debug("selected file: {}", file_name);

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
            eox::xocv::StereoCamera &camera,
            const std::vector<std::filesystem::path> &paths,
            const std::shared_ptr<spdlog::logger> &log
    ) {
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

    void save_calibration_data(
            eox::ocv::StereoPackage &package,
            Gtk::Window &window,
            const data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log
    ) {
        std::string name;
        for (const auto &[id, solo]: package.solo) {
            name += "_" + std::to_string(solo.uid);
        }

        const auto filter_text = Gtk::FileFilter::create();
        filter_text->set_name("Camera calibration files (*.json)");
        filter_text->add_pattern("*.json");

        Gtk::FileChooserDialog dialog("Please select a file to save", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_current_name("camera_" + name + ".json");
        dialog.set_current_folder(configuration.work_dir);
        dialog.add_filter(filter_text);
        dialog.set_transient_for(window);

        dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Save", Gtk::RESPONSE_OK);

        const int result = dialog.run();
        if (result == Gtk::RESPONSE_OK) {

            auto const file_name = dialog.get_filename();
            log->debug("selected file: {}", file_name);

            eox::ocv::write_stereo_package(package, file_name);

            log->debug("calibration data saved");
        } else {
            log->debug("nothing selected");
        }
    }

    std::vector<eox::ocv::StereoPackage> load_calibration_data(
            const std::vector<std::filesystem::path> &paths,
            const std::shared_ptr<spdlog::logger> &log
    ) {
        if (paths.empty()) {
            log->debug("list of config path is empty");
            return {};
        }

        std::vector<eox::ocv::StereoPackage> packages;
        log->debug("load calibration from paths");
        for (const auto &path: paths) {
            const auto ext = path.extension().string();

            log->debug("checking path: {}", path.string());
            if (!std::filesystem::is_regular_file(path))
                continue;

            if (ext == ".json") {
                log->debug("loading configuration: {}", path.string());

                const auto result = eox::ocv::read_stereo_package(path.string());
                if (result.ok) {
                    packages.push_back(result);
                    continue;
                }
            }

            log->debug("file: {}, not a calibration configuration", path.string());
        }

        return packages;
    }

    void init_pre_calibrated_data(
            std::map<uint, eox::ocv::CalibrationSolo> &preCalibrated,
            const std::vector<std::filesystem::path> &paths,
            const eox::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log
    ) {
        const auto props = configuration.camera;
        auto data = load_calibration_data(paths, log);
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

    void init_package_group(
            std::map<uint, eox::ocv::StereoPackage> &packages,
            const std::vector<std::filesystem::path> &paths,
            const eox::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log
    ) {
        log->debug("initializing package groups");
        const auto &groups = configuration.groups;
        auto stereo_data = load_calibration_data(paths, log);
        for (const auto &data: stereo_data) {
            const auto &solo = data.solo;

            // iterate over groups, ie: {1: [4,6,...], 2: [0,2,...]}
            for (const auto &group: groups) {

                // got devices for group, ie: [4,6,...]
                const auto &ids = group.second;
                int matches = 0;

                // iterate over ids, ie: [4,6,...]
                for (const auto &id: ids) {
                    if (solo.contains(id)) {
                        matches++;
                        continue;
                    }
                    break;
                }

                // yep, we found our group
                if (matches == ids.size()) {
                    log->debug("found matching group in stereo config: {}", group.first);
                    packages.emplace(group.first, data);
                    break;
                }
            }
        }
    }

    void read_matcher_data(
            const cv::Ptr<cv::StereoMatcher>& matcher,
            const uint group_id,
            const std::vector<std::filesystem::path> &paths,
            const std::shared_ptr<spdlog::logger> &log
    ) {
        log->debug("reading matcher data from files for group: {}", group_id);
        for (const auto &path: paths) {
            log->debug("checking {} file", path.string());

            if (path.extension().string() != ".json") {
                log->debug("file: {} is not a .json file", path.string());
                continue;
            }

            cv::FileStorage fs(path.string(), cv::FileStorage::READ);

            std::string type;
            fs["type"] >> type;

            if ("eox::group" != type) {
                log->debug("file {} is not a group type", path.string());
                fs.release();
                continue;
            }

            int g_id = -1;
            fs["group_id"] >> g_id;
            if ((int) group_id != g_id) {
                log->debug("{} group id does not match: {}, {}", path.string(), group_id, g_id);
                fs.release();
                continue;
            }

            std::string m_name;
            fs["matcher"] >> m_name;
            fs.release();

            const auto m_path = path.parent_path() / m_name;
            if (eox::ocv::read_stereo_matcher(matcher, m_path)) {
                log->debug("stereo matcher initialized from file: {}", m_path.string());
                return;
            }
        }
        log->debug("No stereo matcher configuration found");
    }

    void read_disparity_filter_data(
            const cv::Ptr<cv::ximgproc::DisparityFilter>& filter,
            const uint group_id,
            const std::vector<std::filesystem::path> &paths,
            const std::shared_ptr<spdlog::logger> &log
    ) {
        log->debug("reading disparity filter data from files for group: {}", group_id);
        for (const auto &path: paths) {
            log->debug("checking {} file", path.string());

            if (path.extension().string() != ".json") {
                log->debug("file: {} is not a .json file", path.string());
                continue;
            }

            cv::FileStorage fs(path.string(), cv::FileStorage::READ);

            std::string type;
            fs["type"] >> type;

            if ("eox::group" != type) {
                log->debug("file {} is not a group type", path.string());
                fs.release();
                continue;
            }

            int g_id = -1;
            fs["group_id"] >> g_id;
            if ((int) group_id != g_id) {
                log->debug("{} group id does not match: {}, {}", path.string(), group_id, g_id);
                fs.release();
                continue;
            }

            std::string f_name;
            fs["filter"] >> f_name;
            fs.release();

            const auto m_path = path.parent_path() / f_name;
            if (eox::ocv::read_disparity_filter(filter, m_path)) {
                log->debug("disparity filter initialized from file: {}", m_path.string());
                return;
            }
        }
        log->debug("No disparity filter configuration found");
    }

    void save_bm_data(
            const cv::Ptr<cv::StereoMatcher>& matcher,
            const cv::Ptr<cv::ximgproc::DisparityFilter>& filter,
            const uint group_id,
            Gtk::Window &window,
            const data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log
    ) {
        log->debug("save group bm data: {}", group_id);
        const auto filter_text = Gtk::FileFilter::create();
        filter_text->set_name("Stereo matcher files (*.json)");
        filter_text->add_pattern("*.json");

        Gtk::FileChooserDialog dialog("Please select a file to save", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_current_name("group_" + std::to_string(group_id) + ".json");
        dialog.set_current_folder(configuration.work_dir);
        dialog.add_filter(filter_text);
        dialog.set_transient_for(window);

        dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Save", Gtk::RESPONSE_OK);

        const int result = dialog.run();
        if (result == Gtk::RESPONSE_OK) {
            auto const file_name = dialog.get_filename();

            const std::filesystem::path parent = dialog.get_file()->get_parent()->get_path();
            const std::string m_name = "matcher_" + std::to_string(group_id) + ".json";
            const std::string f_name = "filter_" + std::to_string(group_id) + ".json";
            cv::FileStorage fs(file_name, cv::FileStorage::WRITE);
            fs << "type" << "eox::group";
            fs << "group_id" << (int) group_id;
            fs << "matcher" << m_name;
            fs << "filter" << f_name;
            fs.release();

            const auto m_path = (parent / m_name).string();
            const auto f_path = (parent / f_name).string();

            eox::ocv::write_stereo_matcher(matcher, m_path);
            eox::ocv::write_disparity_filter(filter, f_path);

            log->debug("group data saved");
        } else {
            log->debug("nothing selected");
        }
    }

    void save_points_ply(
            const eox::ocv::PointCloud &points,
            Gtk::Window &window,
            const eox::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log) {
        log->debug("saving points cloud to ply");

        const auto filter_text = Gtk::FileFilter::create();
        filter_text->set_name("Points cloud files (*.ply)");
        filter_text->add_pattern("*.ply");

        Gtk::FileChooserDialog dialog("Please select a file to save", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_current_folder(configuration.work_dir);
        dialog.set_current_name("points.ply");
        dialog.add_filter(filter_text);
        dialog.set_transient_for(window);

        dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Save", Gtk::RESPONSE_OK);

        if (dialog.run() == Gtk::RESPONSE_OK) {

            auto const file_name = dialog.get_filename();
            log->debug("selected file: {}", file_name);

            std::ofstream file_stream(file_name, std::ios::out);
            if (!file_stream) {
                log->error("File stream opening error");
                // maybe throw some exception or show some modal?
                return;
            }

            // save PLY file
            points.write_to_ply(file_stream);

            // closing the stream
            file_stream.close();
        } else {
            log->debug("nothing selected");
        }
    }

} // events