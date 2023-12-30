//
// Created by henryco on 12/30/23.
//

#include "ui_pose.h"

namespace eox {

    void UiPose::init(eox::data::basic_config configuration) {
        glImage.init(2, 2, 1, 640, 480, {"DEMO"}, GL_BGR);
    }

    void UiPose::update(float delta, float late, float fps) {

    }

    void UiPose::onRefresh() {
        glImage.update();
    }

} // eox