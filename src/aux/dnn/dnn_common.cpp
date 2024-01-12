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
        cv::Mat blob;

        if (in.cols != size || in.rows != size) {

            if (keep_aspect_ratio) {
                // letterbox, preserving aspect ratio
                const float r = (float) in.cols / (float) in.rows;
                const int n_w = size * std::min(1.f, r);
                const int n_h = n_w / std::max(1.f, r);
                const int s_x = (size - n_w) / 2;
                const int s_y = (size - n_h) / 2;

                blob = cv::Mat::zeros(cv::Size(size, size), CV_8UC3);
                cv::Mat roi = blob(cv::Rect(s_x, s_y, n_w, n_h));
                cv::resize(in, roi, cv::Size(n_w, n_h),
                           0, 0, cv::INTER_CUBIC);
            } else {
                // resize without preserving aspect ratio
                cv::resize(in, blob, cv::Size(size, size),
                           0, 0, cv::INTER_CUBIC);
            }

        } else {
            in.copyTo(blob);
        }

        cv::cvtColor(blob, blob, cv::COLOR_BGR2RGB);
        blob.convertTo(blob, CV_32FC3, 1.0 / 255.);

        return blob;
    }


}