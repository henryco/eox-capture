//
// Created by henryco on 1/7/24.
//

#include "pose_roi.h"

#include <cmath>

namespace eox::dnn {

    PoseRoiInput roiFromPoseLandmarks39(const Landmark landmarks[39]) {
        PoseRoiInput output;
        for (int i = 0; i < 39; i++)
            output.landmarks[i] = landmarks[i];
        return output;
    }

    RoI PoseRoi::forward(void *data) {
        return forward(*static_cast<eox::dnn::PoseRoiInput*>(data));
    }

    RoI PoseRoi::forward(const eox::dnn::PoseRoiInput &data) const {
        const auto &center = data.landmarks[33];
        const auto &end = data.landmarks[34];

        const int x1 = center.x;
        const int y1 = center.y;
        const int x2 = end.x;
        const int y2 = end.y;

        const int radius = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2)) + margin;
        const int x0 = x1 - radius;
        const int y0 = y1 - radius;

        return {
                .x = std::max(0, x0),
                .y = std::max(0, y0 + fix_y),
                .w = std::max(0, (radius * 2)),
                .h = std::max(0, (radius * 2)),
        };
    }

    int PoseRoi::getFixY() const {
        return fix_y;
    }

    int PoseRoi::getMargin() const {
        return margin;
    }

    PoseRoi &PoseRoi::setFixY(int fixY) {
        fix_y = fixY;
        return *this;
    }

    PoseRoi &PoseRoi::setMargin(int _margin) {
        margin = _margin;
        return *this;
    }

} // eox