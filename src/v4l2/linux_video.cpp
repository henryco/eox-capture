//
// Created by henryco on 11/25/23.
//

#include "linux_video.h"
#include <linux/videodev2.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>

std::vector<sex::v4l2::V4L2_QueryCtrl> sex::v4l2::get_camera_props(uint id) {
    std::vector<sex::v4l2::V4L2_QueryCtrl> properties;

    const std::string device = "/dev/video" + std::to_string(id);
    const int file_descriptor = open(device.c_str(), O_RDWR);

    if (file_descriptor == -1) {
        std::cerr << "Cannot open video device: " << device << std::endl;
        return {};
    }

    try {

        sex::v4l2::V4L2_QueryCtrl queryctrl;
        queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

        while (0 == ioctl(file_descriptor, VIDIOC_QUERYCTRL, &queryctrl)) {
            if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {

                if (queryctrl.type != V4L2_CTRL_TYPE_CTRL_CLASS) {
                    sex::v4l2::V4L2_Control ctr = {.id = queryctrl.id};
                    if (ioctl(file_descriptor, VIDIOC_G_CTRL, &ctr) == -1) {
                        std::cerr << "Cannot retrieve value for control: " << ctr.id << " [" << device << "]" << std::endl;
                    }
                    queryctrl.value = ctr.value;
                }

                properties.push_back(queryctrl);
            }

            queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }

        if (errno != EINVAL) {
            std::cerr << "fails for reasons other than reaching the end of the control list" << std::endl;
            close(file_descriptor);
            return {};
        }

    } catch (...) {
        close(file_descriptor);
        throw;
    }

    close(file_descriptor);
    return properties;
}

bool sex::v4l2::set_camera_prop(uint device_id, uint prop_id, int prop_value) {
    // not the most efficient way to pass data, but it's ok
    return set_camera_prop(device_id, {
        .id = prop_id,
        .value = prop_value
    });
}

bool sex::v4l2::set_camera_prop(uint device_id, sex::v4l2::V4L2_Control control) {
    // not the most efficient way to pass data, but it's ok.
    return set_camera_prop(
            device_id,
            std::vector<sex::v4l2::V4L2_Control>({control})
            )[0];
}

std::vector<bool> sex::v4l2::set_camera_prop(uint device_id, std::vector<sex::v4l2::V4L2_Control> controls) {
    const std::string device = "/dev/video" + std::to_string(device_id);
    const int file_descriptor = open(device.c_str(), O_RDWR);

    std::vector<bool> results(controls.size());

    if (file_descriptor == -1) {
        std::cerr << "Cannot open video device: " << device << std::endl;
        return results;
    }

    try {

        for (int i = 0; i < controls.size(); ++i) {
            auto& control = controls[i];
            if (ioctl(file_descriptor, VIDIOC_S_CTRL, &control) == -1) {
                std::cerr << "Cannot set control value: " << control.id << std::endl;
                results[i] = false;
            } else {
                results[i] = true;
            }
        }

    } catch (...) {
        close(file_descriptor);
        throw;
    }

    close(file_descriptor);
    return results;
}

void sex::v4l2::reset_defaults(uint device_id) {
    const auto props = get_camera_props(device_id);
    std::vector<sex::v4l2::V4L2_Control> controls;
    controls.reserve(props.size());

    for (const auto &prop: props) {
        controls.push_back({
            .id = prop.id,
            .value = prop.default_value
        });
    }

    set_camera_prop(device_id, controls);
}
