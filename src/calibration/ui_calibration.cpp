//
// Created by henryco on 8/20/23.
//

#include <glibmm/main.h>
#include <iostream>
#include <gtkmm/textview.h>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

void UiCalibration::prepareCamera() {
    const std::vector<int> index = {
            2, //4
    };
    const std::string codec = "YUYV";
    const int width = 640;
    const int height = 480;
    const int fps = 30;

    std::vector<CameraProp> props;
    props.reserve(index.size());
    textures.reserve(index.size());
    glAreas.reserve(index.size());

    int idx = 0;
    for (int i : index) {
        auto prop = CameraProp(i, width, height, codec, fps);

        auto area = std::make_unique<Gtk::GLArea>();
        area->signal_realize().connect(createInitFunc(idx), false);
        area->signal_render().connect(createRenderFunc(idx), false);
        area->set_size_request(prop.width, prop.height);

        h_box.pack_end(*area, Gtk::PACK_SHRINK);
        glAreas.push_back(std::move(area));
        textures.push_back(std::make_unique<xogl::Texture1>());
        props.push_back(prop);
    }

    camera.open(props);
}


void UiCalibration::init() {
    prepareCamera();

    v_box.pack_end(h_box, Gtk::PACK_SHRINK);
    this->set_title("StereoX++ calibration");
    this->set_default_size(1280, 480);
    this->add(v_box);

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &UiCalibration::update), 1000 / 30);
    dispatcher.connect(sigc::mem_fun(*this, &UiCalibration::on_dispatcher_signal));
    show_all_children();
}


std::function<void()> UiCalibration::createInitFunc(const int num) {
    return [num, this]() {
        auto& area = glAreas[num];
        area->make_current();
        area->throw_if_error();
        textures[num]->init();
    };
}

std::function<bool(const Glib::RefPtr<Gdk::GLContext>&)> UiCalibration::createRenderFunc(const int num) {
    return [num, this](const Glib::RefPtr<Gdk::GLContext>& context) -> bool {

        std::cout << "rendering: " << num << std::endl;

        auto& texture = textures[num];
        if (texture == nullptr) {
            return true;
        }

        if (frames.empty()) {
            return true;
        }

        glClearColor(.0f, .0f, .0f, .0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto& frame = frames[num];
        texture->setImage(xogl::Image(frame.data, frame.cols, frame.rows, GL_BGR));
        texture->render();

        return true;
    };
}

bool UiCalibration::update() {
    for (auto &area: glAreas) {
        area->queue_render();
    }
    std::cout << "updated" << std::endl;
    return true;
}

void UiCalibration::loop() {
    // TODO
    dispatcher.emit();
}

void UiCalibration::on_dispatcher_signal() {
    for (auto &area: glAreas) {
        area->queue_render();
    }
}


