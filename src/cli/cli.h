//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_CLI_H
#define STEREOX_CLI_H

#include <opencv2/videoio.hpp>
#include <spdlog/spdlog.h>
#include "../libs/argparse/argparse.h"
#include "../data/common_structures.h"
#include "../utils/globals/sex_globals.h"

namespace sex::cli {

    // Function to parse the devices string into a map
    std::map<unsigned int, unsigned int> parse_devices(const std::vector<std::string> &devices_str_vec) {
        std::map<unsigned int, unsigned int> devices_map;

        for (const auto &devices_str: devices_str_vec) {
            std::istringstream stream(devices_str);
            std::string pair;

            while (std::getline(stream, pair, ',')) {
                std::istringstream pair_stream(pair);
                std::string key_str, value_str;
                if (std::getline(pair_stream, key_str, ':') && std::getline(pair_stream, value_str)) {
                    unsigned int key = std::stoul(key_str);
                    unsigned int value = std::stoul(value_str);
                    devices_map[key] = value;
                }
            }
        }

        return devices_map;
    }

    sex::data::basic_config parse(int argc, char **argv) {
        argparse::ArgumentParser program(
                "stereox",
                "1.0.0",
                argparse::default_arguments::all
        );
        program.add_description(R"desc(
                StereoX - stereo vision modules.

                To list available capture devices on linux you can use:
                [  $ v4l2-ctl --list-devices  ]

                For proper configuration first check your camera allowed properties
                [  $ v4l2-ctl -d \"/dev/video${ID}\" --list-formats-ext  ]
                )desc");

        program.add_argument("module")
                .help("chose the module to run (calibration, vision, config)")
                .required();

        program.add_argument("-o", "--homogeneous")
                .help("enable only if all the video capture devices are of the same model")
                .flag();

        program.add_argument("-j", "--jobs")
                .help("set number of maximum concurrent jobs")
                .default_value(4)
                .scan<'i', int>();


        program.add_argument("-d", "--devices")
                .help("list of devices coma separated (pairs id:index i.e.: '0:2,1:4' )")
                .nargs(argparse::nargs_pattern::any)
                .append();

        program.add_argument("-b", "--buffer")
                .help("set the buffer size")
                .default_value(2)
                .scan<'i', int>();

        program.add_argument("--verbose")
                .help("increase output verbosity")
                .flag();

        program.add_argument("--codec")
                .help("set the video capture codec (4 characters)")
                .default_value(std::string("MJPG"));

        program.add_argument("--width")
                .help("set the width")
                .default_value(640)
                .scan<'i', int>();

        program.add_argument("--height")
                .help("set the height")
                .default_value(480)
                .scan<'i', int>();

        program.add_argument("--fps")
                .help("set the camera maximum frames per second")
                .default_value(30)
                .scan<'i', int>();


        program.add_argument("--fast")
                .help("enable fast mode, faster frame grabbing with the cost of lack of synchronization between devices")
                .flag();

        program.add_argument("--api")
                .help("set the backend API for video capturing (see: cv::CAP_*)")
                .default_value(cv::CAP_V4L2)
                .scan<'i', int>();


        try {
            program.parse_args(argc, argv);
        } catch (const std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            std::cout << program << std::endl;
            std::exit(1);
        }

        sex::globals::THREAD_POOL_CORES_MAX = program.get<int>("--jobs");

        if (program.get<bool>("--verbose")) {
            spdlog::set_level(spdlog::level::debug);
        } else {
            spdlog::set_level(spdlog::level::info);
        }

        const auto codec = program.get<std::string>("--codec");
        const auto devices = parse_devices(
                program.get<std::vector<std::string>>("--devices")
        );

        std::vector<sex::data::camera_properties> props;
        props.reserve(devices.size());
        for (const auto &[id, index]: devices) {
            props.push_back({
                                    .id = id,
                                    .index = index,

                                    .width = program.get<int>("--width"),
                                    .height = program.get<int>("--height"),
                                    .fps = program.get<int>("--fps"),
                                    .buffer = program.get<int>("--buffer"),
                                    .codec = {codec[0],
                                              codec[1],
                                              codec[2],
                                              codec[3]},
                                    .fast = program.get<bool>("--fast"),
                                    .homogeneous = program.get<bool>("--homogeneous"),
                                    .api = program.get<int>("--api")
                            });
        }

        return {
                .camera = props,
                .module = sex::data::enumerated_module.at(program.get<std::string>("module"))
        };
    }

}

#endif //STEREOX_CLI_H
