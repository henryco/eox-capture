//
// Created by henryco on 11/30/23.
//

#ifndef STEREOX_HELPERS_H
#define STEREOX_HELPERS_H

#include <filesystem>
#include <vector>
#include <spdlog/logger.h>

#include "../aux/commons.h"
#include "../aux/ocv/stereo_camera.h"
#include "../aux/ocv/cv_utils.h"

namespace sex::helpers {

    std::vector<std::filesystem::path> config_paths(
            const sex::data::basic_config &configuration);

    std::vector<std::filesystem::path> work_paths(
            const sex::data::basic_config &configuration);

    void gtk_save_camera_settings(
            sex::xocv::StereoCamera &camera,
            const std::vector<uint> &devices,
            Gtk::Window &window,
            const sex::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log
    );

    void load_camera_from_paths(
            sex::xocv::StereoCamera &camera,
            const std::vector<std::filesystem::path> &paths,
            const std::shared_ptr<spdlog::logger> &log);

    void save_calibration_data(
            eox::ocv::StereoPackage &stereoPackage,
            Gtk::Window &window,
            const sex::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log);

    std::vector<eox::ocv::StereoPackage> load_calibration_data(
            const std::vector<std::filesystem::path> &paths,
            const std::shared_ptr<spdlog::logger> &log);

    void init_pre_calibrated_data(
            std::map<uint, eox::ocv::CalibrationSolo> &preCalibrated,
            const std::vector<std::filesystem::path> &paths,
            const sex::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log);

    void init_package_group(
            std::map<uint, eox::ocv::StereoPackage> &packages,
            const std::vector<std::filesystem::path> &paths,
            const sex::data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log);

    void save_bm_data(
            const cv::Ptr<cv::StereoMatcher>& matcher,
            const cv::Ptr<cv::ximgproc::DisparityWLSFilter>& filter,
            const uint group_id,
            Gtk::Window &window,
            const data::basic_config &configuration,
            const std::shared_ptr<spdlog::logger> &log
    );

} // events

#endif //STEREOX_HELPERS_H
