//
// Created by henryco on 1/12/24.
//

#include "ssd_anchors.h"

#include <cmath>

namespace eox::dnn::ssd {

    float calculate_scale(float min_scale, float max_scale, int stride_index, int num_strides) {
        if (num_strides == 1) {
            return (min_scale + max_scale) / 2;
        } else {
            return min_scale + (max_scale - min_scale) * stride_index / (num_strides - 1);
        }
    }

    /**
     * I will not even pretend that I have more than just vague idea of whats going on here
     * https://github.com/google/mediapipe/blob/master/mediapipe/calculators/tflite/ssd_anchors_calculator.cc
     */
    std::vector<std::array<float, 4>> generate_anchors(const SSDAnchorOptions &options) {
        std::vector<std::array<float, 4>> anchors;
        int layer_id = 0;
        int n_strides = options.strides.size();

        while (layer_id < n_strides) {
            std::vector<float> anchor_height;
            std::vector<float> anchor_width;
            std::vector<float> aspect_ratios;
            std::vector<float> scales;
            int last_same_stride_layer = layer_id;

            while (last_same_stride_layer < n_strides &&
                   options.strides[last_same_stride_layer] == options.strides[layer_id]) {
                float scale = calculate_scale(options.min_scale, options.max_scale, last_same_stride_layer, n_strides);

                if (last_same_stride_layer == 0 && options.reduce_boxes_in_lowest_layer) {
                    aspect_ratios.insert(aspect_ratios.end(), {1.0, 2.0, 0.5});
                    scales.insert(scales.end(), {0.1, scale, scale});
                } else {
                    aspect_ratios.insert(aspect_ratios.end(), options.aspect_ratios.begin(),
                                         options.aspect_ratios.end());
                    scales.insert(scales.end(), options.aspect_ratios.size(), scale);

                    if (options.interpolated_scale_aspect_ratio > 0) {
                        float scale_next =
                                last_same_stride_layer == n_strides - 1 ?
                                1.0 : calculate_scale(options.min_scale,
                                                      options.max_scale,
                                                      last_same_stride_layer +
                                                      1, n_strides);
                        scales.push_back(std::sqrt(scale * scale_next));
                        aspect_ratios.push_back(options.interpolated_scale_aspect_ratio);
                    }
                }
                last_same_stride_layer++;
            }

            for (size_t i = 0; i < aspect_ratios.size(); ++i) {
                float ratio_sqrts = std::sqrt(aspect_ratios[i]);
                anchor_height.push_back(scales[i] / ratio_sqrts);
                anchor_width.push_back(scales[i] * ratio_sqrts);
            }

            int stride = options.strides[layer_id];
            int feature_map_height = std::ceil(static_cast<float>(options.input_size_height) / stride);
            int feature_map_width = std::ceil(static_cast<float>(options.input_size_width) / stride);

            for (int y = 0; y < feature_map_height; ++y) {
                for (int x = 0; x < feature_map_width; ++x) {
                    for (size_t anchor_id = 0; anchor_id < anchor_height.size(); ++anchor_id) {
                        float x_center = (x + options.anchor_offset_x) / static_cast<float>(feature_map_width);
                        float y_center = (y + options.anchor_offset_y) / static_cast<float>(feature_map_height);
                        std::array<float, 4> new_anchor = {x_center, y_center, 0.0, 0.0};

                        if (options.fixed_anchor_size) {
                            new_anchor[2] = 1.0;
                            new_anchor[3] = 1.0;
                        } else {
                            new_anchor[2] = anchor_width[anchor_id];
                            new_anchor[3] = anchor_height[anchor_id];
                        }
                        anchors.push_back(new_anchor);
                    }
                }
            }
            layer_id = last_same_stride_layer;
        }
        return anchors;
    }

    std::vector<eox::dnn::DetectedRegion> decode_bboxes(float score_thresh,
                                                        const std::vector<float> &scores,
                                                        const std::vector<std::array<float, 12>> &bboxes,
                                                        const std::vector<std::array<float, 4>> &anchors,
                                                        const float scale,
                                                        bool best_only) {
        // output vector
        std::vector<eox::dnn::DetectedRegion> regions;

        // Sigmoid and thresholding
        std::vector<float> sigmoid_scores(scores.size());
        for (size_t i = 0; i < scores.size(); ++i) {
            sigmoid_scores[i] = eox::dnn::sigmoid(scores[i]);
        }

        std::vector<size_t> valid_indices;
        if (best_only) {
            auto best_it = std::max_element(sigmoid_scores.begin(), sigmoid_scores.end());
            if (*best_it < score_thresh) return regions;
            valid_indices.push_back(std::distance(sigmoid_scores.begin(), best_it));
        } else {
            for (size_t i = 0; i < sigmoid_scores.size(); ++i) {
                if (sigmoid_scores[i] > score_thresh) {
                    valid_indices.push_back(i);
                }
            }
            if (valid_indices.empty()) return regions;
        }

        for (auto idx: valid_indices) {
            std::array<float, 12> det_bbox = bboxes[idx];
            std::array<float, 4> anchor = anchors[idx];

            // Apply scale and anchor adjustments
            for (size_t i = 0; i < det_bbox.size(); i += 2) {
                det_bbox[i] = det_bbox[i] * anchor[2] / scale + anchor[0];
                det_bbox[i + 1] = det_bbox[i + 1] * anchor[3] / scale + anchor[1];
            }

            // Correcting for width and height
            det_bbox[2] -= anchor[0];
            det_bbox[3] -= anchor[1];
            det_bbox[0] -= det_bbox[2] * 0.5;
            det_bbox[1] -= det_bbox[3] * 0.5;

            // Extract key_points
            std::vector<Point> key_points;
            key_points.reserve(4);
            for (int kp = 0; kp < 4; ++kp) {
                key_points.push_back({det_bbox[4 + kp * 2], det_bbox[5 + kp * 2]});
            }

            // Add region
            regions.emplace_back(
                    eox::dnn::Box(
                            det_bbox[0],
                            det_bbox[1],
                            det_bbox[2],
                            det_bbox[3]
                    ),
                    key_points,
                    sigmoid_scores[idx]);
        }

        return regions;
    }
}