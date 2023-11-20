//
// Created by henryco on 8/20/23.
//

#include <glibmm/main.h>
#include <iostream>
#include <gtkmm/textview.h>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

void UiCalibration::init() {

    camera.open({
        CameraProp(4),
//        CameraProp(2)
    });

    this->set_title("StereoX++ calibration");
    this->set_default_size(1280, 720);

    m_GLArea.set_size_request(640, 480);

    m_GLArea.signal_realize().connect(sigc::mem_fun(*this,&UiCalibration::initGl),false);
    m_GLArea.signal_render().connect(sigc::mem_fun(*this,&UiCalibration::on_render),false);
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &UiCalibration::on_timeout), 1000 / 30);

    h_box.pack_start(m_GLArea, Gtk::PACK_SHRINK);
    v_box.pack_start(h_box, Gtk::PACK_SHRINK);

    this->add(v_box);
    show_all_children();
}

void UiCalibration::initGl() {
    m_GLArea.make_current();
    m_GLArea.throw_if_error();
    texture.init();
}

bool UiCalibration::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
    auto frames = camera.capture();
    auto frame = frames[0];
    texture.setImage(xogl::Image(frame.data, frame.cols, frame.rows, GL_BGR));

    glClearColor(.0f, .0f, .0f, .0f);
    glClear(GL_COLOR_BUFFER_BIT);
    texture.render();
    return true;
}

bool UiCalibration::on_timeout() {
    m_GLArea.queue_render();
    return true;
}




