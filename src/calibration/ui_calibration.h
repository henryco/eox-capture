//
// Created by henryco on 8/20/23.
//
#pragma once

#include <gtkmm/window.h>
#include <gtkmm/glarea.h>
#include <GL/gl.h>
#include <gtkmm/box.h>
#include <opencv2/videoio.hpp>

class UiCalibration final : public Gtk::Window {
private:
    cv::VideoCapture cap;
    Gtk::GLArea m_GLArea;
    Gtk::Box m_VBox;
    GLuint texture;

public:
    UiCalibration();

protected:
    bool on_render(const Glib::RefPtr<Gdk::GLContext> &context);
    bool on_timeout();
};