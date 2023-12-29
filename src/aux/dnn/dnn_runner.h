//
// Created by henryco on 12/29/23.
//

#ifndef STEREOX_DNN_RUNNER_H
#define STEREOX_DNN_RUNNER_H

#include <opencv2/dnn/dnn.hpp>

namespace eox::dnn {

    class DnnRunner {
    private:
        std::string file = "../../../models/blazepose_model_float32.pb";
        cv::dnn::Net net = cv::dnn::readNetFromTensorflow(file);


    };

} // eox

#endif //STEREOX_DNN_RUNNER_H
