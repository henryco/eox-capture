//
// Created by henryco on 12/7/23.
//

#include <filesystem>
#include <gtkmm/button.h>
#include "ui_points_cloud.h"
#include "../aux/gtk/gtk_config_stack.h"
#include "../helpers/helpers.h"
#include "../aux/gtk/gtk_cam_params.h"
#include "../aux/utils/mappers/cam_gtk_mapper.h"
#include "../aux/gtk/gtk_utils.h"
#include "../aux/gtk/gtk_control.h"

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
                const auto p = props[0];
                const auto dim = std::to_string(p.output_width) + "x" + std::to_string(p.output_height);
                group_ids.push_back(std::to_string(group.first) + "_1 [" + dim + "]");
                group_ids.push_back(std::to_string(group.first) + "_2 [" + dim + "]");
                group_ids.push_back(std::to_string(group.first) + "_S [" + dim + "]");
            }

            // Init oGL canvas
            glImage.init(2, 2, (int) group_ids.size(), props[0].width, props[0].height, group_ids, GL_BGR);
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
                    camera.resetDefaults(prop.id);
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


            if (config.camera[0].homogeneous) {
                // homogeneous camera configuration, but it applies only for device groups

                const auto controls = camera.getControls(false);
                std::map<uint, sex::xocv::camera_controls> cam_controls;
                for (const auto &control: controls) {
                    cam_controls.emplace(control.id, control);
                }

                for (const auto &[group_id, devices]: groups) {
                    const auto dev_id = devices[0];
                    const auto control = cam_controls.at(dev_id);

                    auto cam_params = std::make_unique<sex::xgtk::GtkCamParams>();
                    cam_params->setProperties(sex::mappers::cam_gtk::map(control.controls));

                    cam_params->onUpdate([devices, this](uint prop_id, int value) {
                        for (const auto &d_id: devices) {
                            log->debug("update camera property: {}, {}, {}", d_id, prop_id, value);
                            camera.setPropValue(d_id, prop_id, value);
                        }
                        return value;
                    });

                    cam_params->onReset([devices, this]() {
                        for (const auto &d_id: devices) {
                            log->debug("resetCamera: {}", d_id);
                            camera.resetDefaults(d_id);
                        }
                    });

                    cam_params->onSave([devices, this]() {
                        log->debug("save camera configuration");
                        std::vector<uint> indexes;
                        indexes.reserve(devices.size());
                        for (const auto &d_id: devices)
                            indexes.push_back(camera.getDeviceIndex(d_id));
                        sex::helpers::gtk_save_camera_settings(camera, indexes, *this, config, log);
                        log->debug("camera configuration done");
                    });

                    config_stack->add(*cam_params, " Group " + std::to_string(group_id));
                    keep(std::move(cam_params));
                }

            } else {
                // non homogeneous camera configuration

                const auto controls = camera.getControls(false);
                for (const auto &control: controls) {
                    const auto dev_id = control.id;

                    auto cam_params = std::make_unique<sex::xgtk::GtkCamParams>();
                    cam_params->setProperties(sex::mappers::cam_gtk::map(control.controls));

                    cam_params->onUpdate([dev_id, this](uint prop_id, int value) {
                        log->debug("update camera property: {}, {}, {}", dev_id, prop_id, value);
                        camera.setPropValue(dev_id, prop_id, value);
                        return value;
                    });

                    cam_params->onReset([dev_id, this]() {
                        log->debug("resetCamera: {}", dev_id);
                        camera.resetDefaults(dev_id);
                    });

                    cam_params->onSave([dev_id, this]() {
                        log->debug("save camera configuration");
                        sex::helpers::gtk_save_camera_settings(camera, {dev_id}, *this, config, log);
                        log->debug("camera configuration done");
                    });

                    config_stack->add(*cam_params, " Camera " + std::to_string(dev_id));
                    keep(std::move(cam_params));
                }
            }
        }

        {

            // each group has its own block matcher
            for (const auto &[group_id, _]: packages) {

                auto scroll_pane = std::make_unique<Gtk::ScrolledWindow>();
                auto v_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
                auto button_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
                v_box->pack_start(*button_box, Gtk::PACK_SHRINK);


                {
                    auto save = std::make_unique<Gtk::Button>();
                    save->get_style_context()->add_class("button-save");
                    save->set_size_request(-1, 30);
                    save->set_label("Save settings");
                    sex::xgtk::add_style(*save, R"css(
                        .button-save {
                             margin-right: 5px;
                             margin-bottom: 5px;
                         }
                    )css");

                    save->signal_clicked().connect([this]() {
                        // TODO on save
                    });

                    button_box->pack_start(*save, Gtk::PACK_SHRINK);
                    keep(std::move(save));
                }


                {
                    auto reset = std::make_unique<Gtk::Button>();
                    reset->set_label("Reset");
                    reset->get_style_context()->add_class("button-reset");
                    reset->set_size_request(-1, 30);
                    sex::xgtk::add_style(*reset, R"css(
                        .button-reset {
                             margin-right: 5px;
                             margin-bottom: 5px;
                         }
                    )css");

                    reset->signal_clicked().connect([this]() {
                        // todo on reset
                    });

                    button_box->pack_start(*reset, Gtk::PACK_SHRINK);
                    keep(std::move(reset));
                }


                // TODO stereo block matcher configuration here
                if (config.stereo.algorithm == sex::data::Algorithm::BM) {
                    log->debug("BM block matcher");

                    auto matcher = cv::StereoBM::create();
                    {
                        // default config here
                        matcher->setPreFilterType(cv::StereoBM::PREFILTER_XSOBEL);
                    }

                    auto right = config.stereo.confidence
                                 ? cv::ximgproc::createRightMatcher(matcher)
                                 : nullptr;
                    const ts::lr_matchers lr_matchers = {.left = matcher, .right = right};
                    matchers.emplace(group_id, lr_matchers);



                }

                else if (config.stereo.algorithm == sex::data::Algorithm::SGBM) {
                    log->debug("SGBM block matcher");

                    auto matcher = cv::StereoSGBM::create();
                    {
                        // default config here
                    }

                    auto right = config.stereo.confidence
                                       ? cv::ximgproc::createRightMatcher(matcher)
                                       : nullptr;
                    const ts::lr_matchers lr_matchers = {.left = matcher, .right = right};
                    matchers.emplace(group_id, lr_matchers);
                }

                else {
                    log->error("Unknown block matcher algorithm");
                    throw std::runtime_error("Unknown block matcher algorithm");
                }

                {
                    auto c_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).left->setNumDisparities((int) value);
                                    return value;
                                }),
                                "NumDisparities",
                                matchers.at(group_id).left->getNumDisparities(),
                                16,
                                64,
                                16,
                                16 * 20
                        );
                        c_box->pack_start(*control);
                        keep(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).left->setBlockSize((int) value);
                                    return value;
                                }),
                                "BlockSize",
                                matchers.at(group_id).left->getBlockSize(),
                                2,
                                21,
                                5,
                                255
                        );
                        c_box->pack_start(*control);
                        keep(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).left->setMinDisparity((int) value);
                                    return value;
                                }),
                                "MinDisparity",
                                matchers.at(group_id).left->getMinDisparity(),
                                1,
                                0,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        keep(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).left->setSpeckleWindowSize((int) value);
                                    return value;
                                }),
                                "SpeckleWindowSize",
                                matchers.at(group_id).left->getSpeckleWindowSize(),
                                1,
                                0,
                                0,
                                255
                        );
                        c_box->pack_start(*control);
                        keep(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).left->setSpeckleRange((int) value);
                                    return value;
                                }),
                                "SpeckleRange",
                                matchers.at(group_id).left->getSpeckleRange(),
                                1,
                                0,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        keep(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).left->setDisp12MaxDiff((int) value);
                                    return value;
                                }),
                                "Disp12MaxDiff",
                                matchers.at(group_id).left->getDisp12MaxDiff(),
                                1,
                                0,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        keep(std::move(control));
                    }

                    v_box->pack_start(*c_box, Gtk::PACK_SHRINK);
                    keep(std::move(c_box));
                }

                {
                    // init wls filter
                    if (config.stereo.confidence) {
                        auto filter = cv::ximgproc::createDisparityWLSFilter(matchers.at(group_id).left);
                        wlsFilters.emplace(group_id, filter);
                    }

                    else {
                        auto filter = cv::ximgproc::createDisparityWLSFilterGeneric(false);
                        wlsFilters.emplace(group_id, filter);
                    }
                }

                scroll_pane->add(*v_box);
                config_stack->add(*scroll_pane, std::to_string(group_id));
                keep(std::move(scroll_pane));
                keep(std::move(button_box));
                keep(std::move(v_box));
            }
        }

        {
            // Init Window
            layout_h->pack_start(glImage, Gtk::PACK_EXPAND_WIDGET);
            layout_h->pack_end(*config_stack, Gtk::PACK_SHRINK);
            add(*layout_h);
            keep(std::move(layout_h));
            keep(std::move(config_stack));
            show_all_children();
        }

        {
            camera.open(false);
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