//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_CLI_H
#define STEREOX_CLI_H

#include <argparse/argparse.hpp>
#include <opencv2/videoio.hpp>
#include <spdlog/spdlog.h>
#include "aux/commons.h"
#include "aux/utils/globals/eox_globals.h"
#include <filesystem>

namespace eox::cli {

    // lower casing strings
    std::string to_lower_case(const std::string &str) {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

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

    std::map<uint, std::vector<uint>> parse_groups(const std::vector<std::string> &groups_str_vec) {
        // input:  1:4,6 2:0,2 aka ["1:4,6", "2:0,2"]
        // output: {1: [4,6], 2: [0,2]}

        std::map<uint, std::vector<uint>> map;
        for (const auto &group_str: groups_str_vec) {

            std::istringstream stream(group_str);
            std::string key, pair;

            while (std::getline(stream, key, ':') && std::getline(stream, pair)) {
                std::istringstream pair_stream(pair);
                std::string value;

                std::vector<uint> values;
                while (std::getline(pair_stream, value, ',')) {
                    values.push_back(std::stoul(value));
                }

                map.emplace(std::stoul(key), std::move(values));
            }
        }

        return map;
    }

    eox::data::basic_config parse(int &argc, char **&argv) {
        argparse::ArgumentParser program(
                "stereox",
                "1.0.0"
        );
        program.add_description(R"desc(
                StereoX - stereo vision modules.

                To list available capture devices on linux you can use:
                [  $ v4l2-ctl --list-devices  ]

                For proper configuration first check your camera allowed properties
                [  $ v4l2-ctl -d \"/dev/video${ID}\" --list-formats-ext  ]
                )desc");

        program.add_argument("-o", "--homogeneous")
                .help("enable only if all the video capture devices are of the same model")
                .flag();
        program.add_argument("-j", "--jobs")
                .help("set number of maximum concurrent jobs")
                .default_value(4)
                .scan<'i', int>();
        program.add_argument("-d", "--device")
                .help("list of devices coma separated (pairs id:index i.e.: '0:2,1:4' )")
                .nargs(argparse::nargs_pattern::any)
                .append();
        program.add_argument("-b", "--buffer")
                .help("set the buffer size")
                .default_value(2)
                .scan<'i', int>();
        program.add_argument("-f", "--config")
                .default_value(std::vector<std::string>{std::filesystem::current_path().string()})
                .help("stereo configuration files or directory")
                .nargs(argparse::nargs_pattern::any)
                .append();
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
        program.add_argument("--output-width")
                .help("set the output width")
                .default_value(0)
                .scan<'i', int>();
        program.add_argument("--output-height")
                .help("set the output height")
                .default_value(0)
                .scan<'i', int>();
        program.add_argument("--scale")
                .help("scale video output (for gui)")
                .default_value(1.0f)
                .scan<'g', float>();
        program.add_argument("--fps")
                .help("set the camera maximum frames per second")
                .default_value(30)
                .scan<'i', int>();
        program.add_argument("--fast")
                .help("enable fast mode, faster frame grabbing with the cost of lack of synchronization between devices")
                .flag();
        program.add_argument("--denoise")
                .help("perform de-noising filter")
                .flag();
        program.add_argument("--api")
                .help("set the backend API for video capturing (see: cv::CAP_*)")
                .default_value((int) cv::CAP_V4L2)
                .scan<'i', int>();


        // Calibration config
        argparse::ArgumentParser calibration("calibration");
        calibration.add_description(R"desc(
                Calibration module, used for stereo camera calibration.
                Use calibration -h for help.
        )desc");
        calibration.add_argument("-r", "--rows")
                .help("number of rows of checkerboard")
                .default_value(7)
                .scan<'i', int>();
        calibration.add_argument("-c", "--columns")
                .help("number of columns of checkerboard")
                .default_value(10)
                .scan<'i', int>();
        calibration.add_argument("-s", "--size")
                .help("square size in cm")
                .default_value(1.0f)
                .scan<'g', float>();
        calibration.add_argument("-q", "--quality")
                .help("quality flags (see cv::CALIB_CB_* enum)")
                .default_value(std::vector<std::string>{"0"})
                .nargs(argparse::nargs_pattern::any)
                .append();
        calibration.add_argument("-n", "--number")
                .help("number of images")
                .default_value(4)
                .scan<'i', int>();
        calibration.add_argument("-y", "--delay")
                .help("delay between shots in ms")
                .default_value(5000)
                .scan<'i', int>();
        calibration.add_argument("-t", "--correction")
                .help("optimize some of all of the camera intrinsic parameters during stereo calibration")
                .flag();
        program.add_subparser(calibration);


        // Stereo config
        argparse::ArgumentParser stereo("stereo");
        stereo.add_description(R"desc(
                Stereo points cloud module.
                Use calibration -h for help.
        )desc");
        stereo.add_argument("--confidence")
                .help("filtering with confidence requires two disparity maps (for the left and right views) "
                      "and is approximately two times slower. However, quality is typically significantly better.")
                .flag();
        stereo.add_argument("-g", "--group")
                .help("list of device groups (pairs id:d1,...,dn i.e.: '1:4,6 2:0,2' )")
                .nargs(argparse::nargs_pattern::any)
                .append();
        stereo.add_argument("-a", "--algorithm")
                .help("pattern matching algorithm [bm, sgbm]")
                .choices("bm", "sgbm")
                .default_value("bm");
        program.add_subparser(stereo);


        // Pose estimation config
        argparse::ArgumentParser pose("pose");
        pose.add_description(R"desc(
                3D pose estimation module.
                Use pose -h for help.
        )desc");
        // TODO
        program.add_subparser(pose);


        try {
            program.parse_args(argc, argv);
        } catch (const std::runtime_error &err) {
            std::cout << err.what() << '\n';
            std::cout << program;
            std::exit(1);
        }

        eox::globals::THREAD_POOL_CORES_MAX = program.get<int>("--jobs");

        if (program.get<bool>("--verbose")) {
            spdlog::set_level(spdlog::level::debug);
        } else {
            spdlog::set_level(spdlog::level::info);
        }

        const auto scale = program.get<float>("--scale");
        const auto codec = program.get<std::string>("--codec");
        const auto devices = parse_devices(
                program.get<std::vector<std::string>>("--device")
        );

        int o_w = program.get<int>("--output-width");
        int o_h = program.get<int>("--output-height");

        if (o_w <= 0)
            o_w = program.get<int>("--width");
        if (o_h <= 0)
            o_h = program.get<int>("--height");

        std::vector<eox::data::camera_properties> props;
        props.reserve(devices.size());
        for (const auto &[id, index]: devices) {
            props.push_back({
                                    .id = id,
                                    .index = index,

                                    .width = program.get<int>("--width"),
                                    .height = program.get<int>("--height"),
                                    .output_width = o_w,
                                    .output_height = o_h,
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

        const auto configs = program.get<std::vector<std::string>>("--config");
        std::vector<std::string> new_configs;
        std::string work_dir;

        for (const auto &path: configs) {
            if (!std::filesystem::exists(path))
                continue;
            if (std::filesystem::is_directory(path)) {
                work_dir = path;
                continue;
            }
            new_configs.push_back(path);
        }

        if (program.is_subcommand_used("calibration")) {
            const auto &instance = program.at<argparse::ArgumentParser>("calibration");

            int quality = 0;
            for (const auto &v: instance.get<std::vector<std::string>>("--quality")) {
                quality |= std::stoi(v);
            }

            return {
                    .denoise = program.get<bool>("--denoise"),
                    .scale = scale,
                    .work_dir = work_dir,
                    .configs = new_configs,
                    .camera = props,
                    .module = "calibration",
                    .calibration = {
                            .columns = instance.get<int>("--columns"),
                            .rows = instance.get<int>("--rows"),
                            .size = instance.get<float>("--size"),
                            .quality = quality,
                            .number = instance.get<int>("--number"),
                            .delay = instance.get<int>("--delay"),
                            .correction = instance.get<bool>("--correction")
                    }
            };
        }

        if (program.is_subcommand_used("stereo")) {
            const auto &instance = program.at<argparse::ArgumentParser>("stereo");

            const auto groups = parse_groups(
                    instance.get<std::vector<std::string>>("--group")
            );

            const auto algo = instance.get<std::string>("--algorithm");

            return {
                    .denoise = program.get<bool>("--denoise"),
                    .scale = scale,
                    .work_dir = work_dir,
                    .configs = new_configs,
                    .camera = props,
                    .groups = groups,
                    .module = "stereo",
                    .stereo = {
                            .algorithm = to_lower_case(algo) == "sgbm"
                                         ? eox::data::Algorithm::SGBM
                                         : eox::data::Algorithm::BM,
                            .confidence = instance.get<bool>("--confidence")
                    }
            };
        }

        if (program.is_subcommand_used("pose")) {
            const auto &instance = program.at<argparse::ArgumentParser>("pose");
            // TODO
            return {
                    .denoise = program.get<bool>("--denoise"),
                    .scale = scale,
                    .work_dir = work_dir,
                    .configs = new_configs,
                    .camera = props,
                    .module = "pose",
            };
        }

        return {
                .denoise = program.get<bool>("--denoise"),
                .scale = scale,
                .work_dir = work_dir,
                .configs = new_configs,
                .camera = props
        };
    }

}

#endif //STEREOX_CLI_H
