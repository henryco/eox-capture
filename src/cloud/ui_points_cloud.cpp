//
// Created by henryco on 12/7/23.
//

#include "ui_points_cloud.h"
#include "../aux/gtk/gtk_config_stack.h"

namespace eox {

    void UiPointsCloud::init(sex::data::basic_config configuration) {
        config = std::move(configuration);

        const auto &groups = config.groups;
        const auto &props = config.camera;
        auto layout_h = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        auto config_stack = std::make_unique<sex::xgtk::GtkConfigStack>();


        {
            // Init oGL canvas
            glImage.init((int) groups.size(), props[0].width, props[0].height, GL_BGR);
            glImage.scale(config.scale);
        }

        {
            // init executor
            executor = std::make_shared<sex::util::ThreadPool>();
            executor->start(props.size());
        }

        {
            // init camera
            camera.setProperties(props);
            camera.setThreadPool(executor);
            camera.setHomogeneous(props[0].homogeneous);
            camera.setFast(props[0].fast);
            camera.setApi(props[0].api);
        }

        {

            // TODO

        }

        {
            // Init Window
            layout_h->pack_start(glImage, Gtk::PACK_SHRINK);
            layout_h->pack_start(*config_stack, Gtk::PACK_SHRINK);
            add(*layout_h);
            keep(std::move(layout_h));
            keep(std::move(config_stack));
            show_all_children();
        }

        {
            camera.open();
        }

        {
            // Stable FPS worker loop
            deltaLoop.setFunc([this](float d, float l, float f) { update(d, l, f); });
            deltaLoop.setFps(0);
            deltaLoop.start();
        }
    }

    void UiPointsCloud::update(float delta, float late, float fps) {
        FPS = fps;
    }

    void UiPointsCloud::onRefresh() {
        set_title("StereoX++ stereo [ " + std::to_string((int) FPS) + " FPS ]");

        // TODO

        glImage.update();
    }

    UiPointsCloud::~UiPointsCloud() {
        log->debug("terminate stereo");

        deltaLoop.stop();
        camera.release();

        log->debug("terminated");
    }


} // eox