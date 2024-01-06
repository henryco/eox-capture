//
// Created by henryco on 1/6/24.
//

#include "low_pass_filter.h"

namespace eox::sig {

    LowPassFilter::LowPassFilter(): initialized(false), last_value(0) {}

    float LowPassFilter::filter(float value, float alpha) {
        if (!initialized) {
            initialized = true;
            last_value = value;
            return last_value;
        }
        last_value = (alpha * value) + ((1.f - alpha) * last_value);
        return last_value;
    }

    float LowPassFilter::getLastValue() const {
        return last_value;
    }

} // eox