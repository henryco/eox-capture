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

#include "../camera/stereo_camera.h"
#include "../ogl/render/texture_1.h"
#include "../gtk/gl_image.h"
#include "../utils/loop/delta_loop.h"
#include "../gtk/gtk_cam_params.h"
#include "../gtk/gtk_config_stack.h"


class UiCalibration final : public Gtk::Window {

public:
    UiCalibration() = default;
    ~UiCalibration() override;
    void init();

protected:
    void on_dispatcher_signal();
    void prepareCamera();
    void update(float delta, float late, float fps);

    std::function<int(uint, int)> updateCamera(std::vector<uint> devices);
    std::function<void()> saveCamera(std::vector<uint> devices);
    std::function<void()> resetCamera(std::vector<uint> devices);

private:
    static inline const auto log =
            spdlog::stdout_color_mt("ui_calibration");

    float FPS = 0;

    Gtk::Box layout_h = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    std::vector<std::unique_ptr<Gtk::Widget>> widgets;

    sex::xgtk::GtkConfigStack configStack;
    sex::xgtk::GLImage glImage;

    sex::xocv::StereoCamera camera;
    sex::util::DeltaLoop deltaLoop;

    Glib::Dispatcher dispatcher;
};
