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
#include "../utils/ogl/render/texture_1.h"
#include "../utils/gtk/gl_image.h"

class UiCalibration final : public Gtk::Window {

public:
    UiCalibration() = default;
    void init();

protected:
    void prepareCamera();
    bool update();

    void on_dispatcher_signal();
    void loop();

private:
    xgtk::GLImage glImage;
    std::vector<cv::Mat> frames;
    Glib::Dispatcher dispatcher;

    StereoCamera camera;
};
