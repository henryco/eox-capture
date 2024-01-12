//
// Created by henryco on 1/4/24.
//

#ifndef STEREOX_DNN_COMMON_H
#define STEREOX_DNN_COMMON_H

#include <vector>

namespace eox::dnn {

    using Point = struct {
        float x, y;
    };

    using RoI = struct {
        int x, y, w, h;
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

        RoI roi;

        std::vector<Point> key_points;

        /**
         * Probability [0,1]
         */
        float score;
    };


    extern const int body_joints[31][2];

    double sigmoid(double x);
}

#endif //STEREOX_DNN_COMMON_H