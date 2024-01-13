//
// Created by henryco on 1/7/24.
//

#ifndef STEREOX_POSE_ROI_H
#define STEREOX_POSE_ROI_H

#include "RoiPredictor.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace eox::dnn {

    using PoseRoiInput = struct {
        eox::dnn::Landmark landmarks[39];
    };

    PoseRoiInput roiFromPoseLandmarks39(const Landmark landmarks[39]);

    PoseRoiInput roiFromPoints(const float mid[2], const float end[2]);

    class PoseRoi : eox::dnn::RoiPredictor {

        static inline const auto log =
                spdlog::stdout_color_mt("pose_roi_predictor");

    private:
        int fix_y = 10;
        int margin = 10;

    public:
        RoI forward(void *data) override;

        [[nodiscard]] RoI forward(const PoseRoiInput &data) const;

        [[nodiscard]] int getMargin() const;

        [[nodiscard]] int getFixY() const;

        PoseRoi &setMargin(int margin);

        PoseRoi &setFixY(int fixY);
    };

} // eox

#endif //STEREOX_POSE_ROI_H
