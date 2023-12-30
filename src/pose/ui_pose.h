//
// Created by henryco on 12/30/23.
//

#ifndef STEREOX_UI_POSE_H
#define STEREOX_UI_POSE_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../aux/gtk/gtk_eox_window.h"
#include "../aux/dnn/blaze_pose.h"
#include "../aux/gtk/gl_image.h"

namespace eox {

    class UiPose : public eox::xgtk::GtkEoxWindow {

        static inline const auto log =
                spdlog::stdout_color_mt("ui_pose");

    private:
        eox::xgtk::GLImage glImage;
        eox::dnn::BlazePose pose;

        float FPS = 0;

    public:
        void init(eox::data::basic_config configuration) override;

        void update(float delta, float late, float fps);

    protected:
        void onRefresh() override;

    };

} // eox

#endif //STEREOX_UI_POSE_H
