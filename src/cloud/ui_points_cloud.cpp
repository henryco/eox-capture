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

                const auto _controls = camera.getControls(false);
                std::map<uint, sex::xocv::camera_controls> cam_controls;
                for (const auto &control: _controls) {
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

                const auto _controls = camera.getControls(false);
                for (const auto &control: _controls) {
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
                auto c_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);

                auto button_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
                v_box->pack_start(*button_box, Gtk::PACK_SHRINK);

                {
                    auto reset = std::make_unique<Gtk::Button>();
                    reset->set_label("Matcher Defaults");
                    reset->get_style_context()->add_class("button-reset");
                    reset->set_size_request(-1, 30);
                    sex::xgtk::add_style(*reset, R"css(
                        .button-reset {
                             margin-right: 5px;
                             margin-bottom: 5px;
                         }
                    )css");

                    reset->signal_clicked().connect([this]() {
                        log->debug("reset stereo matcher controls");
                        for (const auto &control: controls)
                            control->reset();
                    });

                    button_box->pack_start(*reset, Gtk::PACK_SHRINK);
                    keep(std::move(reset));
                }

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

                    save->signal_clicked().connect([this, group_id]() {
                        sex::helpers::save_bm_data(
                                matchers.at(group_id).first,
                                wlsFilters.at(group_id),
                                group_id,
                                *this,
                                config,
                                log
                        );
                    });

                    button_box->pack_start(*save, Gtk::PACK_SHRINK);
                    keep(std::move(save));
                }

                if (config.stereo.algorithm == sex::data::Algorithm::BM) {
                    log->debug("BM block matcher");

                    auto matcher = cv::StereoBM::create();
                    matcher->setTextureThreshold(0);

                    // TODO READ CONFIG FROM FILE

                    std::pair<cv::Ptr<cv::StereoMatcher>, cv::Ptr<cv::StereoMatcher>> lr_matchers(matcher, matcher);
                    matchers.emplace(group_id, std::move(lr_matchers));

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setBlockSize((int) value);
                                    return value;
                                }),
                                "BlockSize",
                                matchers.at(group_id).second->getBlockSize(),
                                2,
                                21,
                                5,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setNumDisparities((int) value);
                                    return value;
                                }),
                                "NumDisparities",
                                matchers.at(group_id).second->getNumDisparities(),
                                16,
                                64,
                                16,
                                16 * 20
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([matcher](double value){
                                    matcher->setPreFilterType((int) value);
                                    return value;
                                }),
                                "PreFilterType",
                                matcher->getPreFilterType(),
                                1,
                                1,
                                0,
                                1
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setPreFilterSize((int) value);
                                    return value;
                                },
                                "PreFilterSize",
                                matcher->getPreFilterSize(),
                                2,
                                9,
                                5,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setPreFilterCap((int) value);
                                    return value;
                                },
                                "PreFilterCap",
                                matcher->getPreFilterCap(),
                                1,
                                31,
                                1,
                                63
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setTextureThreshold((int) value);
                                    return value;
                                },
                                "TextureThreshold",
                                matcher->getTextureThreshold(),
                                1,
                                0,
                                0,
                                300
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setUniquenessRatio((int) value);
                                    return value;
                                },
                                "UniquenessRatio",
                                matcher->getUniquenessRatio(),
                                1,
                                15,
                                0,
                                512
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setSmallerBlockSize((int) value);
                                    return value;
                                },
                                "SmallerBlockSize",
                                matcher->getSmallerBlockSize(),
                                1,
                                0,
                                0,
                                512
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }
                }

                else if (config.stereo.algorithm == sex::data::Algorithm::SGBM) {
                    log->debug("SGBM block matcher");

                    auto matcher = cv::StereoSGBM::create();

                    // TODO READ CONFIG FROM FILE

                    std::pair<cv::Ptr<cv::StereoMatcher>, cv::Ptr<cv::StereoMatcher>> lr_matchers(matcher, matcher);
                    matchers.emplace(group_id, std::move(lr_matchers));

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setBlockSize((int) value);
                                    return value;
                                }),
                                "BlockSize",
                                matchers.at(group_id).second->getBlockSize(),
                                2,
                                3,
                                1,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setNumDisparities((int) value);
                                    return value;
                                }),
                                "NumDisparities",
                                matchers.at(group_id).second->getNumDisparities(),
                                16,
                                64,
                                16,
                                16 * 20
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([matcher](double value){
                                    matcher->setPreFilterCap((int) value);
                                    return value;
                                }),
                                "PreFilterCap",
                                matcher->getPreFilterCap(),
                                1,
                                0,
                                0,
                                100
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setUniquenessRatio((int) value);
                                    return value;
                                },
                                "UniquenessRatio",
                                matcher->getUniquenessRatio(),
                                1,
                                0,
                                0,
                                512
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setP1((int) value);
                                    return value;
                                },
                                "P1",
                                matcher->getP1(),
                                1,
                                0,
                                0,
                                5000
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setP2((int) value);
                                    return value;
                                },
                                "P2",
                                matcher->getP2(),
                                1,
                                0,
                                0,
                                5000
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                [matcher](double value) {
                                    matcher->setMode((int) value);
                                    return value;
                                },
                                "Mode",
                                matcher->getMode(),
                                1,
                                0,
                                0,
                                3
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }
                }

                else {
                    log->error("Unknown block matcher algorithm");
                    throw std::runtime_error("Unknown block matcher algorithm");
                }

                {

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setMinDisparity((int) value);
                                    return value;
                                }),
                                "MinDisparity",
                                matchers.at(group_id).second->getMinDisparity(),
                                1,
                                0,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setSpeckleWindowSize((int) value);
                                    return value;
                                }),
                                "SpeckleWindowSize",
                                matchers.at(group_id).second->getSpeckleWindowSize(),
                                1,
                                0,
                                0,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setSpeckleRange((int) value);
                                    return value;
                                }),
                                "SpeckleRange",
                                matchers.at(group_id).second->getSpeckleRange(),
                                1,
                                0,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([this, group_id](double value){
                                    matchers.at(group_id).first->setDisp12MaxDiff((int) value);
                                    return value;
                                }),
                                "Disp12MaxDiff",
                                matchers.at(group_id).second->getDisp12MaxDiff(),
                                1,
                                0,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }
                }

                {
                    // init wls filter
                    cv::Ptr<cv::ximgproc::DisparityWLSFilter> filter;
                    if (config.stereo.confidence) {
                        filter = cv::ximgproc::createDisparityWLSFilter(matchers.at(group_id).first);
                        wlsFilters.emplace(group_id, filter);
                    }

                    else {
                        filter = cv::ximgproc::createDisparityWLSFilterGeneric(false);
                        wlsFilters.emplace(group_id, filter);
                    }

                    {
                        // TODO READ CONFIG FROM FILE
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([filter](double value){
                                    filter->setLambda(value);
                                    return value;
                                }),
                                "[WLSF] Lambda",
                                filter->getLambda(),
                                0.1,
                                8000,
                                0,
                                16000
                        );
                        control->digits(1);
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([filter](double value){
                                    filter->setSigmaColor(value);
                                    return value;
                                }),
                                "[WLSF] SigmaColor",
                                filter->getSigmaColor(),
                                0.01,
                                1.0,
                                0,
                                100
                        );
                        control->digits(2);
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    if (config.stereo.confidence) {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([filter](double value){
                                    filter->setLRCthresh((int) value);
                                    return value;
                                }),
                                "[WLSF] LRCthresh ",
                                filter->getLRCthresh(),
                                1,
                                24,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }

                    if (config.stereo.confidence) {
                        auto control = std::make_unique<eox::gtk::GtkControl>(
                                ([filter](double value){
                                    filter->setDepthDiscontinuityRadius((int) value);
                                    return value;
                                }),
                                "[WLSF] DepthDiscontinuityRadius",
                                filter->getDepthDiscontinuityRadius(),
                                1,
                                5,
                                -255,
                                255
                        );
                        c_box->pack_start(*control);
                        controls.push_back(std::move(control));
                    }
                }

                v_box->pack_start(*c_box, Gtk::PACK_SHRINK);
                scroll_pane->add(*v_box);
                config_stack->add(*scroll_pane, std::to_string(group_id));
                keep(std::move(scroll_pane));
                keep(std::move(button_box));
                keep(std::move(v_box));
                keep(std::move(c_box));
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
        const std::string name = config.stereo.algorithm == sex::data::BM ? "BM" : "SGBM";
        set_title("StereoX++ " + name + " [ " + std::to_string((int) FPS) + " FPS ]");
        glImage.update();
    }

    UiPointsCloud::~UiPointsCloud() {
        log->debug("terminate stereo");

        deltaLoop.stop();
        camera.release();

        log->debug("terminated");
    }


} // eox