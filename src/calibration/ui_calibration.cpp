//
// Created by henryco on 8/20/23.
//

#include <glibmm/main.h>
#include <iostream>
#include "ui_calibration.h"
#include <GL/gl.h>

void UiCalibration::init() {
    camera.open({
        CameraProp(4),
        CameraProp(2)
    });

    this->set_title("StereoX++ calibration");
    this->set_default_size(648, 480);
    this->add(m_VBox);

    m_VBox.pack_start(m_GLArea);
    m_GLArea.signal_render().connect(
            sigc::mem_fun(
                    *this,
                    &UiCalibration::on_render
                    ),
                    false);

    this->initGl();

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &UiCalibration::on_timeout), 1000 / 30);

    m_GLArea.show();
    m_VBox.show();
}

void UiCalibration::initGl() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool UiCalibration::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
    std::cout << "render" << std::endl;

    const auto frames = camera.capture();
    cv::Mat frame = frames[0];
    cv::Mat second = frames[1];

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        frame.cols,
        frame.rows,
        0,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        frame.ptr());

    std::cout << "w1: " << frame.cols << " h1: " << frame.rows << std::endl;
    std::cout << "w2: " << second.cols << " h2: " << second.rows << std::endl;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    return true;
}

bool UiCalibration::on_timeout() {
    m_GLArea.queue_render();
    return true;
}




