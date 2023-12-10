//
// Created by henryco on 12/7/23.
//

#include <filesystem>
#include "ui_points_cloud.h"
#include "../aux/gtk/gtk_config_stack.h"
#include "../aux/v4l2/linux_video.h"
#include "../helpers/helpers.h"

namespace eox {

    void UiPointsCloud::init(sex::data::basic_config configuration) {
        config = std::move(configuration);

        const auto &groups = config.groups;
        const auto &props = config.camera;
        auto layout_h = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        auto config_stack = std::make_unique<sex::xgtk::GtkConfigStack>();

        {
            std::vector<std::string> group_ids;
            group_ids.reserve((int) groups.size() + (groups.size() * 2));
            for (const auto &group: config.groups) {
                group_ids.push_back(std::to_string(group.first) + "_1");
                group_ids.push_back(std::to_string(group.first) + "_2");
                group_ids.push_back(std::to_string(group.first) + "_S");
            }

            // Init oGL canvas
            glImage.init((int) group_ids.size(), props[0].width, props[0].height, group_ids, GL_BGR);
//            glImage.init((int) groups.size(), props[0].width, props[0].height, GL_BGR);
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

            {
                log->debug("using camera hardware defaults");
                for (const auto &prop: props) {
                    sex::v4l2::reset_defaults(prop.index);
                }
            }

            {
                log->debug("initializing from work directory implicitly");
                const auto paths = sex::helpers::work_paths(config);
                sex::helpers::load_camera_from_paths(camera, paths, log);
                sex::helpers::init_package_group(packages, paths, config, log);
            }

            {
                log->debug("initializing from configuration files explicitly");
                const auto paths = sex::helpers::config_paths(config);
                sex::helpers::load_camera_from_paths(camera, paths, log);
                sex::helpers::init_package_group(packages, paths, config, log);
            }

            if (packages.size() < config.groups.size()) {
                log->error("stereo group number mismatch, probably devices in group lacks of stereo-config file");
                throw std::runtime_error("stereo group number mismatch");
            }

            for (const auto &[id, package]: packages) {
                for (const auto &[_, solo]: package.solo)
                    deviceGroupMap.emplace(solo.uid, id);
            }
        }

        {
            // init block matcher

            wlsFilter = cv::ximgproc::createDisparityWLSFilterGeneric(false);

            // TODO stereo block matcher configuration here
            if (config.stereo.algorithm == sex::data::Algorithm::BM) {
                blockMatcher = cv::StereoBM::create(16*4, 21);

                auto bm = static_pointer_cast<cv::StereoBM>(blockMatcher);
                bm->setPreFilterType(cv::StereoBM::PREFILTER_XSOBEL);
                bm->setPreFilterSize(9);
                bm->setPreFilterCap(31);
                bm->setTextureThreshold(10);
                bm->setUniquenessRatio(15);
                bm->setSpeckleWindowSize(100);
                bm->setSpeckleRange(32);
                bm->setDisp12MaxDiff(1);
            }

            else if (config.stereo.algorithm == sex::data::Algorithm::SGBM)
                blockMatcher = cv::StereoSGBM::create();

            else throw std::runtime_error("Unknown block matcher algorithm");


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