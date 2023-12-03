//
// Created by henryco on 8/20/23.
//
#pragma once

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../aux/ocv/stereo_camera.h"
#include "../aux/gtk/gl_image.h"
#include "../aux/utils/loop/delta_loop.h"
#include "../aux/gtk/gtk_sex_window.h"
#include "../aux/commons.h"
#include "../aux/utils/timer/timer.h"


class UiCalibration final : public sex::xgtk::GtkSexWindow {

private:
    static inline const auto log =
            spdlog::stdout_color_mt("ui_calibration");

    std::shared_ptr<sex::util::ThreadPool> executor;
    sex::data::basic_config config;
    sex::xocv::StereoCamera camera;
    sex::util::DeltaLoop deltaLoop;
    sex::xgtk::GLImage glImage;
    eox::utils::Timer timer;

    float FPS = 0;

public:
    UiCalibration() = default;
    ~UiCalibration() override;

    void init(sex::data::basic_config configuration) override;

protected:
    void onRefresh() override;

    void update(float delta, float late, float fps);

    std::function<int(uint, int)> updateCamera(std::vector<uint> devices);

    std::function<void()> saveCamera(std::vector<uint> devices);

    std::function<void()> resetCamera(std::vector<uint> devices);

};
