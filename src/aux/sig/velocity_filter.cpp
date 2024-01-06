//
// Created by henryco on 1/6/24.
//

#include "velocity_filter.h"

#include <cmath>

namespace eox::sig {

    VelocityFilter::VelocityFilter(int w_size, float v_scale, int fps)
            : max_valid_total_duration(1000000000 / fps), velocity_scale(v_scale), window_size(w_size) {
    }

    float VelocityFilter::filter(std::chrono::nanoseconds timestamp, float value, float scale) {

        float alpha = NAN;

        if (window.empty()) {
            alpha = 1.0;

        } else {
            const auto &last = window.back();

            const auto distance = scale * (value - last.distance);
            const auto duration = timestamp.count() - last.duration;

            int64_t total_duration = duration;
            float total_distance = distance;

            const int64_t max_cumulative_duration = max_valid_total_duration * (1 + window.size());
            for (const auto &element: window) {
                if (total_duration + element.duration > max_cumulative_duration)
                    break;
                total_distance += element.distance;
                total_duration += element.duration;
            }

            const float velocity = total_distance / (total_duration * 1e-9);
            alpha = 1.0f - 1.0f / (1.0f + velocity_scale * std::abs(velocity));

            window.push_front({distance, duration});
            if (window.size() > window_size) {
                window.pop_back();
            }
        }

        return low_pass.filter(value, alpha);
    }

    void VelocityFilter::reset() {
        window.clear();
    }

    void VelocityFilter::setTargetFps(int fps) {
        max_valid_total_duration = 1000000000 / fps;
        reset();
    }

    void VelocityFilter::setWindowSize(int size) {
        window_size = size;
        reset();
    }

    void VelocityFilter::setVelocityScale(float scale) {
        velocity_scale = scale;
        reset();
    }

} // eox