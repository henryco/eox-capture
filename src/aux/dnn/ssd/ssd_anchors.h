//
// Created by henryco on 1/12/24.
//

#ifndef STEREOX_SSD_ANCHORS_H
#define STEREOX_SSD_ANCHORS_H

#include <vector>
#include <array>
#include "../dnn_common.h"

namespace eox::dnn::ssd {

    struct SSDAnchorOptions {
        int num_layers;
        float min_scale;
        float max_scale;
        int input_size_height;
        int input_size_width;
        float anchor_offset_x;
        float anchor_offset_y;
        std::vector<int> strides;
        std::vector<float> aspect_ratios;
        bool reduce_boxes_in_lowest_layer;
        float interpolated_scale_aspect_ratio;
        bool fixed_anchor_size;
    };

    std::vector<std::array<float, 4>> generate_anchors(const SSDAnchorOptions& options);

    std::vector<eox::dnn::DetectedRegion> decode_bboxes(float score_thresh,
                                                        const std::vector<float> &scores,
                                                        const std::vector<std::array<float, 12>> &bboxes,
                                                        const std::vector<std::array<float, 4>> &anchors,
                                                        const float scale = 224.f,
                                                        bool best_only = false);
}

#endif //STEREOX_SSD_ANCHORS_H
