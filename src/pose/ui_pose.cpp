//
// Created by henryco on 12/30/23.
//

#include "ui_pose.h"

#include <opencv2/imgcodecs.hpp>

namespace eox {

    void UiPose::init(eox::data::basic_config configuration) {

        const auto image = cv::imread("./../media/pose.png", cv::IMREAD_COLOR);
//        const auto image = cv::imread("/home/henryco/Pictures/nino.png", cv::IMREAD_COLOR);
//        const auto image = cv::imread("/home/henryco/Pictures/rosemi.png", cv::IMREAD_COLOR);
        cv::resize(image, frame, cv::Size(640, 480));

        {
            glImage.init(1, 1, 1, 640, 480, {"DEMO"}, GL_BGR);
            glImage.setFrame(frame);
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
            deltaLoop.setFps(144);
            deltaLoop.start();
        }
    }

    void UiPose::update(float _delta, float _late, float _fps) {
        this->FPS = _fps;
        // TODO
        auto result = pose.inference(frame);

        if (result.presence > 0.7) {
            cv::Mat segmentation(128, 128, CV_32F, result.segmentation.data());

            cv::Mat segmentation_mask;
            cv::threshold(segmentation, segmentation_mask, 0.5, 1., cv::THRESH_BINARY);
            cv::resize(segmentation_mask, segmentation_mask, cv::Size(640, 480));
            segmentation_mask.convertTo(segmentation_mask, CV_32FC1, 255.);
            segmentation_mask.convertTo(segmentation_mask, CV_8UC1);

            cv::Mat output;
            cv::bitwise_and(frame, frame, output, segmentation_mask);

            glImage.setFrame(output);
        } else {
            glImage.setFrame(frame);
        }

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