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

class UiCalibration final : public Gtk::Window {

public:
    UiCalibration() = default;
    void init();

protected:
    void prepareCamera();
    bool update();

    void on_dispatcher_signal();
    void loop();

    std::function<bool(const Glib::RefPtr<Gdk::GLContext> &)> createRenderFunc(int num);
    std::function<void()> createInitFunc(int num);

private:
    std::vector<std::unique_ptr<xogl::Texture1>> textures;
    std::vector<std::unique_ptr<Gtk::GLArea>> glAreas;
    std::vector<cv::Mat> frames;

    Gtk::Box h_box = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box v_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);

    Glib::Dispatcher dispatcher;

    StereoCamera camera;
};
