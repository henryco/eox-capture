//
// Created by henryco on 12/30/23.
//

#include "ui_pose.h"

#include <opencv2/imgcodecs.hpp>

namespace eox {

    void UiPose::init(eox::data::basic_config configuration) {
        const auto image = cv::imread("./../media/pose2.png", cv::IMREAD_COLOR);
//        const auto image = cv::imread("/home/henryco/Pictures/nino.png", cv::IMREAD_COLOR);
//        const auto image = cv::imread("/home/henryco/Pictures/rosemi.png", cv::IMREAD_COLOR);
        cv::resize(image, frame, cv::Size(640, 480));

        {
            pipeline.setThreshold(0.5f);
            pipeline.init();
        }

        {
            glImage.init(1, 1, 1, 640, 480, {"DEMO"}, GL_BGR);
            glImage.setFrame(frame);
            glImage.scale(2);
        }

        {
            // Init Window
            auto layout_h = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
            layout_h->pack_start(glImage, Gtk::PACK_EXPAND_WIDGET);
            add(*layout_h);
            show_all_children();
        }

        {
            // Stable FPS worker loop
            deltaLoop.setFunc([this](float d, float l, float f) { update(d, l, f); });
            deltaLoop.setFps(30);
            deltaLoop.start();
        }
    }

    void UiPose::update(float _delta, float _late, float _fps) {
        this->FPS = _fps;
        // TODO

        cv::Mat output, segmentation;
        pipeline.pass(frame, segmentation, output);
        glImage.setFrame(output);

        refresh();
    }

    void UiPose::onRefresh() {
        set_title("StereoX++ pose estimation [ " + std::to_string((int) FPS) + " FPS ]");
        glImage.update();
    }

    UiPose::~UiPose() {
        log->debug("terminate pose");
        deltaLoop.stop();
        log->debug("terminated");
    }

} // eox