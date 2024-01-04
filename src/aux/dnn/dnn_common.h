//
// Created by henryco on 1/4/24.
//

#ifndef STEREOX_DNN_COMMON_H
#define STEREOX_DNN_COMMON_H

namespace eox::dnn {
    using Landmark = struct {

        /**
         * X normalized, [0,1]
         */
        float x;

        /**
         * Y normalized, [0,1]
         */
        float y;

        /**
         * Z normalized, [0,1]
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
}

#endif //STEREOX_DNN_COMMON_H