//
// Created by henryco on 1/7/24.
//

#include "pose_roi.h"

#include <cmath>

namespace eox::dnn {

    RoI PoseRoi::forward(void *data) {
        return forward(*static_cast<eox::dnn::PoseRoiInput*>(data));
    }

    RoI PoseRoi::forward(const eox::dnn::PoseRoiInput &data) const {

        constexpr int FIX_Y = 10;
        constexpr int MARGIN = 10;

        const auto &center = data.pose.landmarks_norm[33];
        const auto &end = data.pose.landmarks_norm[34];

        // global coordinates
        const int x1 = (center.x * data.origin_roi.w) + data.origin_roi.x;
        const int y1 = (center.y * data.origin_roi.h) + data.origin_roi.y;
        const int x2 = (end.x * data.origin_roi.w) + data.origin_roi.x;
        const int y2 = (end.y * data.origin_roi.h) + data.origin_roi.y;

        const int radius = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2)) + MARGIN;
        const int x0 = x1 - radius;
        const int y0 = y1 - radius;

        return {
                .x = std::max(0, x0),
                .y = std::max(0, y0 + FIX_Y),
                .w = std::max(0, (radius * 2)),
                .h = std::max(0, (radius * 2)),
        };
    }
} // eox