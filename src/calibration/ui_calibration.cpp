//
// Created by henryco on 8/20/23.
//

#include <glibmm/main.h>
#include <iostream>
#include <gtkmm/textview.h>
#include "ui_calibration.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../utils/stb_image.h"
#include "../v4l2/linux_video.h"

void UiCalibration::prepareCamera() {

    // TEMPORAL
    const std::vector<int> index = {
//            0,
//            2,
            4
    };
    const std::string codec = "MJPG";
    const int width = 640;
    const int height = 480;
    const int fps = 30;
    // TEMPORAL

    std::vector<sex::CameraProp> props;
    props.reserve(index.size());
    for (int i: index) {
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

//        glImage.init((int) index.size(), min_w, min_h, GL_BGR);
        glImage.init(1, min_w * (int) index.size(), min_h, GL_BGR);

        camera.open(props);

        deltaLoop = std::make_unique<sex::DeltaLoop>(
                [this](float d, float l, float f) { update(d, l, f); });
    }

    {
        auto v4_props = sex::v4l2::get_camera_props(4);
        std::vector<sex::xgtk::GtkCamProp> parameters;
        parameters.reserve(v4_props.size());
        for (const auto &p: v4_props) {
            // TODO
            parameters.emplace_back(
                    p.id,
                    p.type,
                    std::string(reinterpret_cast<const char*>(p.name), 32),
                    p.minimum,
                    p.maximum,
                    p.step,
                    p.default_value,
                    0
            );
        }

        camParams.setProperties(parameters);
    }
}

void UiCalibration::init() {
    prepareCamera();

    set_title("StereoX++ calibration");
    add(layout_h);

    layout_h.pack_start(glImage, Gtk::PACK_SHRINK);
    layout_h.pack_start(camParams, Gtk::PACK_SHRINK);

    dispatcher.connect(sigc::mem_fun(*this, &UiCalibration::on_dispatcher_signal));
    show_all_children();
}

void UiCalibration::update(float delta, float latency, float fps) {
    log->info("update: {}, late: {}, fps: {}", delta, latency, fps);

    auto captured = camera.capture();
    if (captured.empty()) {
        log->debug("skip");
        return;
    }



    // TODO FIXME =====
    auto first = captured[0];
    cv::Mat merged(first.rows, (int) (first.cols * captured.size()), first.type());
    for (int i = 0; i < captured.size(); i++) {
        auto source = captured[i];
        source.copyTo(merged(cv::Rect(source.cols * i, 0, source.cols, source.rows)));
    }
    std::vector<cv::Mat> vec = {merged};
    glImage.setFrames(std::move(vec));
    // TODO FIXME =====



//    glImage.setFrames(std::move(captured));
    dispatcher.emit();
}

void UiCalibration::on_dispatcher_signal() {
    glImage.update();
}

UiCalibration::~UiCalibration() {
    log->debug("terminate calibration");

    deltaLoop->stop();
    camera.release();

    log->debug("terminated");
}


