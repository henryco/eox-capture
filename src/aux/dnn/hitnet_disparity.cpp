//
// Created by henryco on 1/18/24.
//

#include <opencv2/core.hpp>
#include "hitnet_disparity.h"
#include "dnn_common.h"

namespace eox::dnn {

    namespace htn {
        float *disparity_ref(const tflite::Interpreter &interpreter) {
            return interpreter.output_tensor(0)->data.f;
        }

        float *disparity_sec(const tflite::Interpreter &interpreter) {
            return interpreter.output_tensor(1)->data.f;
        }
    }

    std::string HitnetDisparity::get_model_file() {
        return "./../models/hitnet/hitnet_" +
               std::to_string(width) + "x" + std::to_string(height) +
               "_float32.tflite";
    }

    HitnetOutput HitnetDisparity::inference(const cv::_InputArray &left, const cv::_InputArray &right) {
        cv::Mat blob_l = eox::dnn::convert_to_squared_blob(
                left.getMat(),
                getWidth(),
                getHeight(),
                true
        );
        cv::Mat blob_r = eox::dnn::convert_to_squared_blob(
                right.getMat(),
                getWidth(),
                getHeight(),
                true
        );

        constexpr int from_to[] = {
                0, 0,
                1, 1,
                2, 2,
                0, 3,
                1, 4,
                2, 5
        };
        cv::Mat in[] = {blob_l, blob_r};

        cv::Mat blob(blob_l.rows, blob_l.cols, CV_MAKETYPE(blob_l.depth(), 6));
        cv::mixChannels(in, 2, &blob, 1, from_to, 6);

        return inference(blob.ptr<float>(0));
    }

    HitnetOutput HitnetDisparity::inference(const float *frame) {
        init();

        input(0, frame, width * height * 4 * 6);
        invoke();

        auto disparity = htn::disparity_ref(*interpreter);
        cv::Mat mat(width, height, CV_32FC1, disparity);
        mat.convertTo(mat, CV_32FC1, 255);
        mat.convertTo(mat, CV_8UC1);

        return {
            .disparity = mat
        };
    }

    size_t HitnetDisparity::getWidth() const {
        return width;
    }

    size_t HitnetDisparity::getHeight() const {
        return height;
    }

    void HitnetDisparity::setWidth(size_t _width) {
        width = _width;
    }

    void HitnetDisparity::setHeight(size_t _height) {
        height = _height;
    }

} // eox