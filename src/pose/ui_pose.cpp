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
        auto result = pose.inference(frame);

        const float threshold = 0.0;
        if (result.presence > 0.7) {
            cv::Mat segmentation(128, 128, CV_32F, result.segmentation.data());

            cv::Mat segmentation_mask;
            cv::threshold(segmentation, segmentation_mask, 0.5, 1., cv::THRESH_BINARY);
            cv::resize(segmentation_mask, segmentation_mask, cv::Size(640, 480));
            segmentation_mask.convertTo(segmentation_mask, CV_32FC1, 255.);
            segmentation_mask.convertTo(segmentation_mask, CV_8UC1);

            cv::Mat output;
            cv::bitwise_and(frame, frame, output, segmentation_mask);


            for (auto bone : eox::dnn::body_joints) {

                const auto &start = result.landmarks_norm.at(bone[0]);
                const auto &end = result.landmarks_norm.at(bone[1]);

                if (eox::dnn::sigmoid(start.p) > threshold && eox::dnn::sigmoid(end.p) > threshold) {
                    cv::Point sp(start.x * output.cols, start.y * output.rows);
                    cv::Point ep(end.x * output.cols, end.y * output.rows);
                    cv::Scalar color(230, 0, 230);

                    cv::line(output, sp, ep, color, 2);
                }
            }

            int i = 0;
            for (const auto &point: result.landmarks_norm) {
                if (eox::dnn::sigmoid(point.v) > threshold) {
                    cv::Point circle(point.x * output.cols, point.y * output.rows);
                    cv::Scalar color(0, 255, 230);

                    cv::circle(output, circle, 2, color, 1);

                    if (i > 32) {
                        cv::putText(output, std::to_string(i), cv::Point(circle.x - 10, circle.y - 10),
                                    cv::FONT_HERSHEY_PLAIN, 1,
                                    cv::Scalar(255, 0, 0)
                        );
                    }

                    i++;
                }
            }

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