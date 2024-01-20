//
// Created by henryco on 12/29/23.
//

#include "blaze_pose.h"
#include <filesystem>

namespace eox::dnn {

    const float *lm_3d_1x195(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(0)->data.f;
    }

    const float *lm_world_1x117(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(1)->data.f;
    }

    const float *heatmap_1x64x64x39(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(2)->data.f;
    }

    const float *segmentation_1x128x128x1(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(3)->data.f;
    }

    const float *pose_flag_1x1(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(4)->data.f;
    }

//    const std::vector<std::string> BlazePose::outputs = {
//            "Identity:0",   // 598 | 0: [1, 195]           landmarks 3d
//            "Identity_4:0", // 600 | 1: [1, 117]           world 3d
//            "Identity_3:0", // 481 | 2: [1, 64, 64, 39]    heatmap
//            "Identity_2:0", // 608 | 3: [1, 128, 128, 1]   segmentation
//            "Identity_1:0", // 603 | 4: [1, 1]             pose flag (score)
//    };

    PoseOutput BlazePose::inference(cv::InputArray &frame) {
        // [1, 3, 256, 256]
        cv::Mat blob = eox::dnn::convert_to_squared_blob(frame.getMat(), in_resolution);
        return inference(blob.ptr<float>(0));
    }

    PoseOutput BlazePose::inference(const float *frame) {
        init();

        input(0, frame, in_resolution * in_resolution * 3 * 4);
        invoke();

        PoseOutput output;

        const auto presence = *pose_flag_1x1(*interpreter);
        output.score = presence;

        const float *land_marks_3d = lm_3d_1x195(*interpreter);
        const float *land_marks_wd = lm_world_1x117(*interpreter);

        for (int i = 0; i < 39; i++) {
            const int j = i * 3;
            const int k = i * 5;
            // normalized landmarks_3d
            output.landmarks_norm[i] = {
                    .x = land_marks_3d[k + 0] / (float) in_resolution,
                    .y = land_marks_3d[k + 1] / (float) in_resolution,
                    .z = land_marks_3d[k + 2] / (float) in_resolution,
                    .v = land_marks_3d[k + 3],
                    .p = land_marks_3d[k + 4],
            };

            // world-space landmarks
            output.landmarks_3d[i] = {
                    .x = land_marks_wd[j + 0],
                    .y = land_marks_wd[j + 1],
                    .z = land_marks_wd[j + 2],
            };
        }

        const float *s = segmentation_1x128x128x1(*interpreter);
        for (int i = 0; i < 128 * 128; i++) {
            output.segmentation[i] = eox::dnn::sigmoid(s[i]);
        }
        return output;
    }

    std::string BlazePose::get_model_file() {
        return "./../models/blazepose/blazepose_heavy_float32.tflite";
    }

} // eox