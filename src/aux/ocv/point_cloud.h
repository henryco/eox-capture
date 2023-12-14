//
// Created by henryco on 12/14/23.
//

#ifndef STEREOX_POINT_CLOUD_H
#define STEREOX_POINT_CLOUD_H

#include <opencv2/core/mat.hpp>
#include <utility>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace eox::ocv {
    class PointCloud {

        static inline const auto log =
                spdlog::stdout_color_mt("point_cloud");

    public:
        cv::UMat disparities;
        cv::UMat points;
        cv::UMat colors;

        PointCloud() = default;

        PointCloud(
                cv::UMat disparities,
                cv::UMat points,
                cv::UMat colors);

        void write_to_ply(std::ostream &out) const;
    };
} // eox

#endif //STEREOX_POINT_CLOUD_H
