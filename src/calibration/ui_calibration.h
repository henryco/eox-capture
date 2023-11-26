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
#include <gtkmm/stack.h>
#include <gtkmm/stackswitcher.h>

#include "../camera/stereo_camera.h"
#include "../ogl/render/texture_1.h"
#include "../gtk/gl_image.h"
#include "../utils/loop/delta_loop.h"
#include "../gtk/gtk_cam_params.h"


class UiCalibration final : public Gtk::Window {

public:
    UiCalibration() = default;
    ~UiCalibration() override;
    void init();

protected:
    void on_dispatcher_signal();
    void prepareCamera();
    void update(float delta, float late, float fps);

    std::function<int(uint, int)> updateCamera(uint num);

private:
    static inline const auto log =
            spdlog::stdout_color_mt("ui_calibration");

    float FPS = 0;

    Gtk::Box layout_h = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box layout_v = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    std::vector<std::unique_ptr<Gtk::Widget>> widgets;
    sex::xgtk::GLImage glImage;

    std::unique_ptr<sex::DeltaLoop> deltaLoop;
    sex::xocv::StereoCamera camera;

    std::vector<sigc::connection> debouncers;
    Glib::Dispatcher dispatcher;
};
