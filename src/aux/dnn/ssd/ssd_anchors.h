//
// Created by henryco on 1/12/24.
//

#ifndef STEREOX_SSD_ANCHORS_H
#define STEREOX_SSD_ANCHORS_H

#include <vector>
#include <array>

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

    float calculate_scale(float min_scale, float max_scale, int stride_index, int num_strides);

    std::vector<std::array<float, 4>> generate_anchors(const SSDAnchorOptions& options);

}

#endif //STEREOX_SSD_ANCHORS_H
