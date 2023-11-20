//
// Created by henryco on 8/20/23.
//
#pragma once

#include <gtkmm/window.h>
#include <gtkmm/glarea.h>
#include <GL/gl.h>
#include <gtkmm/box.h>

#include "../camera/stereo_camera.h"
#include "../utils/ogl/render/texture_1.h"

class UiCalibration final : public Gtk::Window {

public:
    UiCalibration() = default;
    void init();

protected:
    bool on_render(const Glib::RefPtr<Gdk::GLContext> &context);
    bool on_timeout();
    void initGl();

private:
    Gtk::GLArea m_GLArea;
    Gtk::Box h_box = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box v_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);

    xogl::Texture1 texture;
    StereoCamera camera;
};
