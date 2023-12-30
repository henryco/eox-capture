//
// Created by henryco on 8/20/23.
//
#pragma once

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>

#include "../aux/ocv/stereo_camera.h"
#include "../aux/gtk/gl_image.h"
#include "../aux/utils/loop/delta_loop.h"
#include "../aux/gtk/gtk_eox_window.h"
#include "../aux/commons.h"
#include "../aux/utils/timer/timer.h"
#include "../aux/ocv/cv_utils.h"

namespace eox {

    class UiCalibration final : public eox::xgtk::GtkEoxWindow {

        static inline const auto log =
                spdlog::stdout_color_mt("ui_calibration");

    private:
        std::shared_ptr<eox::util::ThreadPool> executor;
        eox::data::basic_config config;
        eox::xocv::StereoCamera camera;
        eox::util::DeltaLoop deltaLoop;
        eox::xgtk::GLImage glImage;
        eox::utils::Timer timer;

        double progress = 0;
        bool active = false;
        float FPS = 0;

        Gtk::ProgressBar progressBar;
        Gtk::Button start;
        Gtk::Button save;

        eox::ocv::StereoPackage stereoPackage;
        std::map<uint, eox::ocv::CalibrationSolo> preCalibrated;
        std::map<uint, std::vector<std::vector<cv::Point2f>>> image_points;
        int cap = 0;

    public:
        UiCalibration() = default;
        ~UiCalibration() override;

        void init(eox::data::basic_config configuration) override;

    protected:
        void onRefresh() override;

        void update(float delta, float late, float fps);

        std::function<int(uint, int)> updateCamera(std::vector<uint> devices);

        std::function<void()> saveCamera(std::vector<uint> devices);

        std::function<void()> resetCamera(std::vector<uint> devices);

        void update_ui(int remains, std::vector<cv::Mat>& _frames);
    };
}