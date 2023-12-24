//
// Created by henryco on 12/7/23.
//

#ifndef STEREOX_UI_POINTS_CLOUD_H
#define STEREOX_UI_POINTS_CLOUD_H

#include <opencv2/ximgproc/disparity_filter.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <opencv2/core/ocl.hpp>
#include "../aux/gtk/gtk_eox_window.h"
#include "../aux/ocv/stereo_camera.h"
#include "../aux/utils/loop/delta_loop.h"
#include "../aux/gtk/gl_image.h"
#include "../aux/ocv/cv_utils.h"
#include "../aux/gtk/gtk_control.h"
#include "../aux/ocv/point_cloud.h"

namespace eox {

    class UiPointsCloud : public eox::xgtk::GtkEoxWindow { // NOLINT(*-special-member-functions)

        static inline const auto log =
                spdlog::stdout_color_mt("ui_cloud");

    private:
        std::shared_ptr<eox::util::ThreadPool> executor;
        eox::data::basic_config config;
        eox::xocv::StereoCamera camera;
        eox::util::DeltaLoop deltaLoop;
        eox::xgtk::GLImage glImage;

        // map of group -> disparity filter
        std::map<ts::group_id, cv::Ptr<cv::ximgproc::DisparityWLSFilter>> wlsFilters;

        // map of group -> Left and Right stereo matchers pair
        std::map<ts::group_id, std::pair<cv::Ptr<cv::StereoMatcher>, cv::Ptr<cv::StereoMatcher>>> matchers;

        // map of group -> stereo camera configuration
        std::map<ts::group_id, eox::ocv::StereoPackage> packages;

        // map of device -> group
        std::map<ts::device_id, ts::group_id> deviceGroupMap;

        // map of group -> point_cloud
        std::map<ts::group_id, eox::ocv::PointCloud> points;

        std::vector<std::unique_ptr<eox::gtk::GtkControl>> controls;
        float FPS = 0;

        bool aux = true;

    public:
        UiPointsCloud() = default;
        ~UiPointsCloud() override;

        void init(eox::data::basic_config configuration) override;

        void update(float delta, float late, float fps);

    protected:
        void onRefresh() override;

        // TODO
    };

} // eox

#endif //STEREOX_UI_POINTS_CLOUD_H
