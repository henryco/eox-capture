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
        eox::dnn::PoseOutput pose;
        eox::dnn::RoI origin_roi;
        int origin_w;
        int origin_h;
    };

    class PoseRoi : eox::dnn::RoiPredictor {

        static inline const auto log =
                spdlog::stdout_color_mt("pose_roi_predictor");

    public:
        RoI forward(void *data) override;

        [[nodiscard]] RoI forward(const PoseRoiInput &data) const;
    };

} // eox

#endif //STEREOX_POSE_ROI_H
