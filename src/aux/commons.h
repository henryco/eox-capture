//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_COMMONS_H
#define STEREOX_COMMONS_H

namespace sex::data {

    typedef enum  {
        SGBM,
        BM
    } Algorithm;

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
        int columns;
        int rows;
        float size;
        int quality;
        int number;
        int delay;
        bool correction;
    } calibration_config;

    typedef struct {
        Algorithm algorithm;
    } stereo_config;

    typedef struct {
        float scale;
        std::string work_dir;
        std::vector<std::string> configs;
        std::vector<camera_properties> camera;
        std::map<uint, std::vector<uint>> groups;
        std::string module;
        union {
            calibration_config calibration;
            stereo_config stereo;
        };
    } basic_config;
}

#endif //STEREOX_COMMONS_H
