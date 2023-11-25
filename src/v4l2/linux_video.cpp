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


std::vector<sex::v4l2::V4L2_QueryCtrl> sex::v4l2::get_camera_props(int id) {
    std::vector<sex::v4l2::V4L2_QueryCtrl> properties;

    const std::string device = "/dev/video" + std::to_string(id);
    const int file_descriptor = open(device.c_str(), O_RDWR);

    if (file_descriptor == -1) {
        std::cerr << "Cannot open video device: " << device << std::endl;
        return {};
    }

    try {
        sex::v4l2::V4L2_QueryCtrl queryctrl = {.id = V4L2_CTRL_FLAG_NEXT_CTRL};

        while (0 == ioctl(file_descriptor, VIDIOC_QUERYCTRL, &queryctrl)) {
            if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {
                properties.push_back(queryctrl);
            }
            queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }

        if (errno != EINVAL) {
            close(file_descriptor);
            std::cerr << "fails for reasons other than reaching the end of the control list" << std::endl;
            return {};
        }

    } catch (...) {
        close(file_descriptor);
        throw;
    }

    close(file_descriptor);
    return properties;
}
