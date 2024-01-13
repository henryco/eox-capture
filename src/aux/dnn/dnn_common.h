//
// Created by henryco on 1/4/24.
//

#ifndef STEREOX_DNN_COMMON_H
#define STEREOX_DNN_COMMON_H

#include <vector>
#include <opencv2/core/mat.hpp>

namespace eox::dnn {

    using Paddings = struct {
        float left;
        float right;
        float top;
        float bottom;
    };

    using Point = struct {
        float x, y;
    };

    using RoI = struct {
        float x, y, w, h;
    };

    using Landmark = struct {

        /**
         * X
         */
        float x;

        /**
         * Y
         */
        float y;

        /**
         * Z
         */
        float z;

        /**
         * visibility (need to apply sigmoid)
         */
        float v;

        /**
         * presence (need to apply sigmoid)
         */
        float p;
    };

    using PoseOutput = struct {

        /**
         * 39x5 normalized (0,1) landmarks
         */
        eox::dnn::Landmark landmarks_norm[39];

        /**
         * 1D 128x128 float32 array
         */
        float segmentation[128 * 128];

        /**
         * Probability [0,1]
         */
        float score;
    };

    using DetectedRegion = struct {

        /**
         * Detected SSD box
         */
        RoI box;

        /**
         * Key point 0 - mid hip center
         * Key point 1 - point that encodes size & rotation (for full body)
         * Key point 2 - mid shoulder center
         * Key point 3 - point that encodes size & rotation (for upper body)
         */
        std::vector<Point> key_points;

        /**
         * Probability [0,1]
         */
        float score;

        /**
         * from -Pi to Pi radians
         */
        float rotation;

        float width;
        float height;
        float center_x;
        float center_y;
    };


    extern const int body_joints[31][2];

    double sigmoid(double x);

    Paddings get_letterbox_paddings(int width, int height, int size);

    cv::Mat convert_to_squared_blob(const cv::Mat &in, int size, bool keep_aspect_ratio = false);

}

#endif //STEREOX_DNN_COMMON_H