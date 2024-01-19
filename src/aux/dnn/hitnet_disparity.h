//
// Created by henryco on 1/18/24.
//

#ifndef STEREOX_HITNET_DISPARITY_H
#define STEREOX_HITNET_DISPARITY_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "dnn_runner.h"

namespace eox::dnn {

    typedef enum {
        Float_16 = 16,
        Float_32 = 32
    } Precision;

    using HitNetOutput = struct {
        cv::Mat disparity;
    };

    class HitNetDisparity : DnnRunner<HitNetOutput> {
        static inline const auto log =
                spdlog::stdout_color_mt("hitnet_disparity");

        std::string root_dir = "./../models/hitnet";
        Precision precision = Float_16;
        size_t width = 640;
        size_t height = 480;

    protected:
        std::string get_model_file() override;

    public:
        HitNetOutput inference(const cv::_InputArray &left, const cv::_InputArray &right);

        HitNetOutput inference(const float *frame) override;

        [[nodiscard]] size_t getWidth() const;

        [[nodiscard]] size_t getHeight() const;

        [[nodiscard]] const std::string &getRootDir() const;

        [[nodiscard]] Precision getPrecision() const;

        void setWidth(size_t width);

        void setHeight(size_t height);

        void setRootDir(const std::string &rootDir);

        void setPrecision(Precision precision);
    };

} // eox

#endif //STEREOX_HITNET_DISPARITY_H
