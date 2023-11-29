//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_DATA_STRUCTURES_H
#define STEREOX_DATA_STRUCTURES_H

namespace sex::data {

    union config_value {
        int integer;
        bool boolean;
        char array[4];
    };
}

#endif //STEREOX_DATA_STRUCTURES_H
