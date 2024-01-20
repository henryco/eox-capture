//
// Created by henryco on 1/5/24.
//

#import "dnn_common.h"

#import <cmath>
#include <opencv2/imgproc.hpp>

namespace eox::dnn {

    /**
     * see media/pose_landmark_topology.svg
     */
    const int body_joints[31][2] = {
            {0,  2},
            {0,  5},
            {2,  7},
            {5,  8},

            {10, 9},

            {12, 11},

            {12, 14},
            {14, 16},
            {16, 22},
            {16, 18},
            {16, 20},
            {18, 20},

            {11, 13},
            {13, 15},
            {15, 21},
            {15, 17},
            {15, 19},
            {17, 19},

            {12, 24},
            {24, 23},
            {11, 23},

            {24, 26},
            {26, 28},
            {28, 32},
            {28, 30},
            {32, 30},

            {23, 25},
            {25, 27},
            {27, 29},
            {27, 31},
            {29, 31},
    };

    double sigmoid(double x) {
        return 1.0 / (1.0 + std::exp(-x));
    }

    cv::Mat convert_to_squared_blob(const cv::Mat &in, int size, bool keep_aspect_ratio) {
        return convert_to_squared_blob(in, size, size, keep_aspect_ratio);
    }

    cv::Mat convert_to_squared_blob(const cv::Mat &in, int width, int height, bool keep_aspect_ratio) {
        cv::Mat blob;

        if (in.cols != width || in.rows != height) {

            if (keep_aspect_ratio) {
                // letterbox, preserving aspect ratio
                const float r = (float) in.cols / (float) in.rows;
                const int n_w = width * std::min(1.f, r);
                const int n_h = n_w / std::max(1.f, r);
                const int s_x = (width - n_w) / 2;
                const int s_y = (height - n_h) / 2;

                blob = cv::Mat::zeros(cv::Size(width, height), CV_8UC3);
                cv::Mat roi = blob(cv::Rect(s_x, s_y, n_w, n_h));
                cv::resize(in, roi, cv::Size(n_w, n_h),
                           0, 0, cv::INTER_CUBIC);
            } else {
                // resize without preserving aspect ratio
                cv::resize(in, blob, cv::Size(width, height),
                           0, 0, cv::INTER_CUBIC);
            }

        } else {
            in.copyTo(blob);
        }

        cv::cvtColor(blob, blob, cv::COLOR_BGR2RGB);
        blob.convertTo(blob, CV_32FC3, 1.0 / 255.);

        return blob;
    }

    Paddings get_letterbox_paddings(int width, int height, int size) {
        return get_letterbox_paddings(width, height, size, size);
    }

    Paddings get_letterbox_paddings(int width, int height, int box_w, int box_h) {
        const float r = (float) width / (float) height;
        const int n_w = box_w * std::min(1.f, r);
        const int n_h = n_w / std::max(1.f, r);
        const int s_x = (box_w - n_w) / 2.f;
        const int s_y = (box_h - n_h) / 2.f;
        return {
                .left = (float) s_x,
                .right = (float) s_x,
                .top = (float) s_y,
                .bottom = (float) s_y,
        };
    }

    cv::Mat remove_paddings(const cv::Mat &in, int width, int height) {
        const auto paddings = get_letterbox_paddings(
                width, height, in.cols, in.rows
        );
        return in(cv::Rect(
                paddings.left,
                paddings.top,
                width - (paddings.left + paddings.right),
                height - (paddings.top + paddings.bottom)
        ));
    }

    RoI clamp_roi(const RoI &in, int width, int height) {
        auto roi = RoI(in.x, in.y, in.w, in.h, in.c, in.e);

//        // clamping roi, not square
//        const int s_x = std::max(0, (int) roi.x);
//        const int s_y = std::max(0, (int) roi.y);
//        const int e_x = std::min(width, (int) roi.w);
//        const int e_y = std::min(height, (int) roi.h);
//
//        // sides of the roi
//        const int a = e_x - s_x;
//        const int b = e_y - s_y;
//
//        // halves of those sizes
//        const int r_x = a / 2;
//        const int r_y = b / 2;
//
//        // current clamped center
//        const int c_x = s_x + r_x;
//        const int c_y = s_y + r_y;
//
//        // size of new square size
//        const int s = std::min(width, height);
//        const int c = std::min(std::max(a, b), s);
//
//        // shift of current center from desired one
//        const int k_x = r_x - (c / 2);
//        const int k_y = r_y - (c / 2);
//
//        // center of new square roi
//        const int nc_x = c_x + k_x;
//        const int nc_y = c_y + k_y;
//
//        // new clamped and squared roi
//        roi.x = (int) (nc_x - c / 2);
//        roi.y = (int) (nc_y - c / 2);
//        roi.w = (int) c;
//        roi.h = (int) c;

        const int end_x = roi.x + roi.w;
        const int end_y = roi.y + roi.h;

        if (end_x > width) {
            roi.x -= (end_x - width);
        }
        if (end_y > height) {
            roi.y -= (end_y - height);
        }

        roi.x = (int) std::max(0.f, roi.x);
        roi.y = (int) std::max(0.f, roi.y);
        roi.w = (int) std::min(width - roi.x, roi.w);
        roi.h = (int) std::min(height - roi.y, roi.h);

        const auto c_x = roi.x + 0.5 * roi.w;
        const auto c_y = roi.y + 0.5 * roi.h;
        const auto c = std::min(roi.w, roi.h);
        roi.x = c_x - 0.5 * c;
        roi.y = c_y - 0.5 * c;
        roi.w = c;
        roi.h = c;
        return roi;
    }

    double normalize_radians(double angle) {
        return angle - 2 * M_PI * floor((angle + M_PI) / (2 * M_PI));
    }

}