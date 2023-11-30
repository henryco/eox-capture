//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_COMMON_STRUCTURES_H
#define STEREOX_COMMON_STRUCTURES_H

namespace sex::data {

    typedef struct {
        uint id;
        uint index;
        int width;
        int height;
        int fps;
        int buffer;
        char codec[4];
        bool fast;
        bool homogeneous;
        int api;
    } camera_properties;


    typedef struct {
        std::vector<camera_properties> camera;
        std::string module;
    } basic_config;
}

#endif //STEREOX_COMMON_STRUCTURES_H
