//
// Created by henryco on 12/8/23.
//

#include "ui_points_cloud.h"

namespace eox {

    void UiPointsCloud::update(float delta, float late, float fps) {
        FPS = fps;

        auto captured = camera.captureWithId();
        if (captured.empty()) {
            log->debug("nothing captured, skip");
            return;
        }



        refresh();
    }

}