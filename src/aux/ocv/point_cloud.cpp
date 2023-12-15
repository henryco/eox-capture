//
// Created by henryco on 12/14/23.
//

#include <opencv2/imgproc.hpp>
#include "point_cloud.h"

namespace eox::ocv {

    PointCloud::PointCloud(cv::UMat disparities, cv::UMat points, cv::UMat colors)
            :
            disparities(std::move(disparities)),
            points(std::move(points)),
            colors(std::move(colors)) {
    }

    void PointCloud::write_to_ply(std::ostream &out) const {

        cv::UMat u_rgb, t_rgb;
        cv::normalize(colors, t_rgb, 0, 255, cv::NORM_MINMAX, CV_8U);
        cv::cvtColor(t_rgb, u_rgb, cv::COLOR_BGR2RGB);

        cv::UMat u_xyz = points.reshape(1, (int) points.total() / 3);
        u_rgb = u_rgb.reshape(1, (int) u_rgb.total() / 3);

        double min_z = -1;
        double max_z = -1;
        cv::minMaxIdx(u_xyz.col(2), &min_z, &max_z);

        cv::Mat filtered_xyz, filtered_rgb, xyz, rgb;
        u_xyz.copyTo(xyz);
        u_rgb.copyTo(rgb);

        for (int i = 0; i < xyz.rows; ++i) {
            if (xyz.at<float>(i, 2) < max_z) {
                filtered_xyz.push_back(xyz.row(i));
                filtered_rgb.push_back(rgb.row(i));
            }
        }

        out << "ply\n";
        out << "format ascii 1.0\n";
        out << "element vertex " << filtered_xyz.rows << "\n";
        out << "property float x\n";
        out << "property float y\n";
        out << "property float z\n";
        out << "property uchar red\n";
        out << "property uchar green\n";
        out << "property uchar blue\n";
        out << "end_header\n";

        for (int i = 0; i < xyz.rows; i++) {
            out << filtered_xyz.at<float>(i, 0) << " ";
            out << filtered_xyz.at<float>(i, 1) << " ";
            out << filtered_xyz.at<float>(i, 2) << " ";
            out << (uint) filtered_rgb.at<uchar>(i, 0) << " ";
            out << (uint) filtered_rgb.at<uchar>(i, 1) << " ";
            out << (uint) filtered_rgb.at<uchar>(i, 2) << "\n";
        }

        out.flush();
    }
} // eox