//
// Created by henryco on 8/20/23.
//
#pragma once

#include <gtkmm/window.h>
#include <gtkmm/glarea.h>
#include <GL/gl.h>
#include <gtkmm/box.h>
#include <glibmm/dispatcher.h>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../aux/ocv/stereo_camera.h"
#include "../aux/ogl/render/texture_1.h"
#include "../aux/gtk/gl_image.h"
#include "../aux/utils/loop/delta_loop.h"
#include "../aux/gtk/gtk_cam_params.h"
#include "../aux/gtk/gtk_config_stack.h"
#include "../aux/gtk/gtk_sex_window.h"
#include "../aux/commons.h"


class UiCalibration final : public sex::xgtk::GtkSexWindow {

private:
    static inline const auto log =
            spdlog::stdout_color_mt("ui_calibration");

    Gtk::Box layout_h = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    sex::xgtk::GtkConfigStack configStack;
    sex::xgtk::GLImage glImage;
    sex::xocv::StereoCamera camera;
    sex::util::DeltaLoop deltaLoop;

    float FPS = 0;

public:
    UiCalibration() = default;
    ~UiCalibration() override;

    void init(const sex::data::basic_config& configuration) override;

protected:
    void onRefresh() override;

    void update(float delta, float late, float fps);

    std::function<int(uint, int)> updateCamera(std::vector<uint> devices);

    std::function<void()> saveCamera(std::vector<uint> devices);

    std::function<void()> resetCamera(std::vector<uint> devices);

};
