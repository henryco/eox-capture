//
// Created by henryco on 12/7/23.
//

#ifndef STEREOX_UI_POINTS_CLOUD_H
#define STEREOX_UI_POINTS_CLOUD_H

#include <opencv2/ximgproc/disparity_filter.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "../aux/gtk/gtk_sex_window.h"
#include "../aux/ocv/stereo_camera.h"
#include "../aux/utils/loop/delta_loop.h"
#include "../aux/gtk/gl_image.h"
#include "../aux/ocv/cv_utils.h"

namespace eox {

    namespace ts {
        using group_id = unsigned int;
        using device_id = unsigned int;
        using Frame = cv::Mat;

        using lr_matchers = struct {
            cv::Ptr<cv::StereoMatcher> left;
            cv::Ptr<cv::StereoMatcher> right;
        };
    }

    class UiPointsCloud : public sex::xgtk::GtkSexWindow { // NOLINT(*-special-member-functions)

        static inline const auto log =
                spdlog::stdout_color_mt("ui_cloud");

    private:
        std::shared_ptr<sex::util::ThreadPool> executor;
        sex::data::basic_config config;
        sex::xocv::StereoCamera camera;
        sex::util::DeltaLoop deltaLoop;
        sex::xgtk::GLImage glImage;

        std::map<ts::group_id, cv::Ptr<cv::ximgproc::DisparityWLSFilter>> wlsFilters;
        std::map<ts::group_id, ts::lr_matchers> matchers;
        std::map<ts::group_id, eox::ocv::StereoPackage> packages;
        std::map<ts::device_id, ts::group_id> deviceGroupMap;

        float FPS = 0;

    public:
        UiPointsCloud() = default;
        ~UiPointsCloud() override;

        void init(sex::data::basic_config configuration) override;

        void update(float delta, float late, float fps);

    protected:
        void onRefresh() override;

        // TODO
    };

} // eox

#endif //STEREOX_UI_POINTS_CLOUD_H
