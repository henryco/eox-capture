//
// Created by henryco on 1/18/24.
//

#ifndef STEREOX_HITNET_DISPARITY_H
#define STEREOX_HITNET_DISPARITY_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "dnn_runner.h"

namespace eox::dnn {

    using HitnetOutput = struct {
        cv::Mat disparity;
    };

    class HitnetDisparity : DnnRunner<HitnetOutput> {
        static inline const auto log =
                spdlog::stdout_color_mt("hitnet_disparity");

        size_t width{};
        size_t height{};

    protected:
        std::string get_model_file() override;

    public:
        HitnetOutput inference(const cv::_InputArray &left, const cv::_InputArray &right);

        HitnetOutput inference(const float *frame) override;

        [[nodiscard]] size_t getWidth() const;

        [[nodiscard]] size_t getHeight() const;

        void setWidth(size_t width);

        void setHeight(size_t height);
    };

} // eox

#endif //STEREOX_HITNET_DISPARITY_H
