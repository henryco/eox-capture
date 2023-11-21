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
    void init();

protected:
    void on_dispatcher_signal();
    void prepareCamera();

    void update(float delta);

private:
    std::unique_ptr<sex::DeltaLoop> deltaLoop;
    std::vector<cv::Mat> frames;
    Glib::Dispatcher dispatcher;
    xgtk::GLImage glImage;
    StereoCamera camera;
};
