//
// Created by henryco on 1/6/24.
//

#ifndef STEREOX_VELOCITY_FILTER_H
#define STEREOX_VELOCITY_FILTER_H

#include "low_pass_filter.h"

#include <chrono>
#include <deque>

namespace eox::sig {

    namespace velocity_filter {
        using Element = struct {
            float distance;
            int64_t duration;
        };
    }

    class VelocityFilter {

    private:
        std::deque<velocity_filter::Element> window;
        eox::sig::LowPassFilter low_pass;
        int64_t max_valid_total_duration;
        float velocity_scale;
        int window_size;

    public:
        VelocityFilter(int window_size, float velocity_scale, int target_fps = 30);

        float filter(std::chrono::nanoseconds timestamp, float value, float scale = 1.0);

        void setVelocityScale(float scale);

        void setWindowSize(int size);

        void setTargetFps(int fps);

        void reset();
    };

} // eox

#endif //STEREOX_VELOCITY_FILTER_H
