//
// Created by henryco on 8/20/23.
//

#include <glibmm/main.h>
#include <iostream>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

void UiCalibration::init() {

//    camera.open({
//        CameraProp(4),
//        CameraProp(2)
//    });

    image = stbi_load("../resources/animu.png", &i_w, &i_h, &i_c, 0);

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
    texture.init();
    texture.setImage(xogl::Image(image, i_w, i_h));
}

bool UiCalibration::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
    std::cout << "render" << std::endl;

    glClearColor(.0f, .0f, .0f, .0f);
    glClear(GL_COLOR_BUFFER_BIT);

    texture.render();

    return true;
}

bool UiCalibration::on_timeout() {
    m_GLArea.queue_render();
    return true;
}




