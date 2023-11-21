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

    // TEMPORAL
    const std::vector<int> index = {
            2, //4
    };
    const std::string codec = "YUYV";
    const int width = 640;
    const int height = 480;
    const int fps = 30;
    // TEMPORAL

    std::vector<CameraProp> props;
    props.reserve(index.size());
    for (int i : index) {
        props.emplace_back(i, width, height, codec, fps);
    }

    {
        // FIND COMMON MIN VALUES
        int min_fps = 0;
        int min_w = 0;
        int min_h = 0;
        for (const auto &prop: props) {
            if (min_fps == 0 || prop.fps < min_fps)
                min_fps = prop.fps;
            if (min_w == 0 || prop.width < min_w)
                min_w = prop.width;
            if (min_h == 0 || prop.height < min_h)
                min_h = prop.height;
        }

        glImage.init((int) index.size(), min_w, min_h, GL_BGR);
        deltaLoop = std::make_unique<sex::DeltaLoop>(
                [this](float d, float l) { update(d, l); },
                min_fps);
    }

    camera.open(props);
}


void UiCalibration::init() {
    prepareCamera();

    this->set_title("StereoX++ calibration");
    this->set_default_size(1280, 480);
    this->add(glImage);

    dispatcher.connect(sigc::mem_fun(*this, &UiCalibration::on_dispatcher_signal));
    show_all_children();
}

std::vector<std::shared_ptr<cv::Mat>> steal(std::vector<cv::Mat> &captured) {
    std::vector<std::shared_ptr<cv::Mat>> images;
    images.reserve(captured.size());
    for (auto& mat: captured)
        images.push_back(std::make_shared<cv::Mat>(std::move(mat)));
    return images;
}

void UiCalibration::update(float delta, float late) {
    std::cout << "LOOP: " << delta << " LATE: " << late << "\n" << std::endl;

    auto captured = camera.capture();
//    auto images = steal(captured);

    // TODO SOME LOGIC

    dispatcher.emit();
}

void UiCalibration::on_dispatcher_signal() {
    glImage.update();
}


