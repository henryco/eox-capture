//
// Created by henryco on 8/20/23.
//

#include <glibmm/main.h>
#include <iostream>
#include "ui_calibration.h"
#include <GL/gl.h>

UiCalibration::UiCalibration() {
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
    m_GLArea.show();
    m_VBox.show();

    cap.open(0);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera\n";
        return;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &UiCalibration::on_timeout), 1000 / 30);

}

bool UiCalibration::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
    cv::Mat frame;
    cap >> frame;

    if (frame.empty()) {
        std::cerr << "Failed to capture frame\n";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    cv::Mat flippedImage;
    cv::flip(frame, flippedImage, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, flippedImage.cols, flippedImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, flippedImage.ptr());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    return true;
}

bool UiCalibration::on_timeout() {
    m_GLArea.queue_render();
    return true;
}




