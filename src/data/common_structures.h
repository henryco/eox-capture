//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_COMMON_STRUCTURES_H
#define STEREOX_COMMON_STRUCTURES_H

namespace sex::data {

    enum module {
        calibration,
        // todo more
    };

    const std::map<std::string, module> enumerated_module = {
            {"calibration", module::calibration},
            // todo more modules
    };

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
        module module;
    } basic_config;
}

#endif //STEREOX_COMMON_STRUCTURES_H
