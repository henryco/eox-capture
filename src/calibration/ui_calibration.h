//
// Created by henryco on 8/20/23.
//
#pragma once

#include <gtkmm/window.h>
#include <gtkmm/glarea.h>
#include <GL/gl.h>
#include <gtkmm/box.h>
#include <glibmm/dispatcher.h>

#include "../camera/stereo_camera.h"
#include "../ogl/render/texture_1.h"
#include "../gtk/gl_image.h"
#include "../utils/loop/delta_loop.h"

class UiCalibration final : public Gtk::Window {

public:
    UiCalibration() = default;
    ~UiCalibration() override;
    void init();

protected:
    void on_dispatcher_signal();
    void prepareCamera();

    void update(float delta, float late, float fps);

private:
    static inline const auto log =
            spdlog::stdout_color_mt("ui_calibration");

    std::unique_ptr<sex::DeltaLoop> deltaLoop;
    Glib::Dispatcher dispatcher;
    xgtk::GLImage glImage;
    sex::StereoCamera camera;
};
