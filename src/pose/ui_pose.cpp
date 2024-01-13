//
// Created by henryco on 12/30/23.
//

#include "ui_pose.h"
#include "../aux/gtk/gtk_control.h"
#include "../aux/gtk/gtk_config_stack.h"

#include <opencv2/imgcodecs.hpp>
#include <gtkmm/scrolledwindow.h>

namespace eox {

    void UiPose::init(eox::data::basic_config configuration) {
//        const auto image = cv::imread("./../media/pose4.png", cv::IMREAD_COLOR);
        const auto image = cv::imread("./../media/pose3.png", cv::IMREAD_COLOR);
//        const auto image = cv::imread("./../media/pose2.png", cv::IMREAD_COLOR);
//        const auto image = cv::imread("/home/henryco/Pictures/nino.png", cv::IMREAD_COLOR);
//        const auto image = cv::imread("/home/henryco/Pictures/rosemi.png", cv::IMREAD_COLOR);
        cv::resize(image, frame, cv::Size(640, 480));

        {
            pipeline.setDetectorThreshold(0.5f);
            pipeline.setPoseThreshold(0.5f);
            pipeline.init();
        }

        {
            glImage.init(1, 1, 1, 640, 480, {"DEMO"}, GL_BGR);
            glImage.setFrame(frame);
            glImage.scale(configuration.scale);
        }

        {
            auto config_stack = Gtk::make_managed<eox::xgtk::GtkConfigStack>();
            auto scroll_pane = Gtk::make_managed<Gtk::ScrolledWindow>();
            auto control_box_v = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
            auto control_box_h = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);

            config_stack->set_size_request(420);

            config_stack->add(*scroll_pane, "Pose");
            scroll_pane->add(*control_box_h);
            control_box_h->pack_start(*control_box_v, Gtk::PACK_SHRINK);

            {
                auto control = Gtk::make_managed<eox::gtk::GtkControl>(
                        ([this](double value) {
                            pipeline.setPoseThreshold((float) value);
                            return value;
                        }),
                        "PoseThreshold",
                        pipeline.getPoseThreshold(),
                        0.01,
                        0.5,
                        0.0,
                        1.0
                );
                control->digits(2);

                auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
                box->set_size_request(400, 50);
                box->pack_start(*control);
                control_box_v->pack_start(*box, Gtk::PACK_SHRINK);
            }

            {
                auto control = Gtk::make_managed<eox::gtk::GtkControl>(
                        ([this](double value) {
                            pipeline.setDetectorThreshold((float) value);
                            return value;
                        }),
                        "DetectorThreshold",
                        pipeline.getDetectorThreshold(),
                        0.01,
                        0.5,
                        0.0,
                        1.0
                );
                control->digits(2);

                auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
                box->set_size_request(400, 50);
                box->pack_start(*control);
                control_box_v->pack_start(*box, Gtk::PACK_SHRINK);
            }

            {
                auto control = Gtk::make_managed<eox::gtk::GtkControl>(
                        ([this](double value) {
                            pipeline.setFilterVelocityScale((float) value);
                            return value;
                        }),
                        "FilterVelocityScale",
                        pipeline.getFilterVelocityScale(),
                        0.01,
                        0.5,
                        0,
                        100
                );
                control->digits(2);

                auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
                box->set_size_request(400, 50);
                box->pack_start(*control);
                control_box_v->pack_start(*box, Gtk::PACK_SHRINK);
            }

            {
                auto control = Gtk::make_managed<eox::gtk::GtkControl>(
                        ([this](double value) {
                            pipeline.setFilterWindowSize((int) value);
                            return value;
                        }),
                        "FilterWindowSize",
                        pipeline.getFilterWindowSize(),
                        1,
                        30,
                        1,
                        300
                );

                auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
                box->set_size_request(400, 50);
                box->pack_start(*control);
                control_box_v->pack_start(*box, Gtk::PACK_SHRINK);
            }

            {
                auto control = Gtk::make_managed<eox::gtk::GtkControl>(
                        ([this](double value) {
                            pipeline.setFilterTargetFps((int) value);
                            return value;
                        }),
                        "FilterTargetFPS",
                        pipeline.getFilterTargetFps(),
                        1,
                        30,
                        1,
                        300
                );

                auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
                box->set_size_request(400, 50);
                box->pack_start(*control);
                control_box_v->pack_start(*box, Gtk::PACK_SHRINK);
            }

            // Init Window
            auto layout_h = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
            layout_h->pack_start(glImage, Gtk::PACK_EXPAND_WIDGET);
            layout_h->pack_start(*config_stack, Gtk::PACK_SHRINK);

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