//
// Created by henryco on 1/7/24.
//

#ifndef STEREOX_ROIPREDICTOR_H
#define STEREOX_ROIPREDICTOR_H

#include "../dnn_common.h"

namespace eox::dnn {
    class RoiPredictor {

    public:
        virtual eox::dnn::RoI forward(void* data) = 0;
    };
}

#endif //STEREOX_ROIPREDICTOR_H
