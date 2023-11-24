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

void sex::v4l2::grab_camera_props(int id) {

    const std::string device = "/dev/video" + std::to_string(id);
    const int file_descriptor = open(device.c_str(), O_RDWR);

    if (file_descriptor == -1) {
        std::cerr << "Cannot open video device: " << device << std::endl;
        return;
    }

    try {
        struct v4l2_queryctrl queryctrl = {};
        queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

        std::cout << "Controls for device " << device << ":" << std::endl;

        while (0 == ioctl(file_descriptor, VIDIOC_QUERYCTRL, &queryctrl)) {
            if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {
                std::cout << "Control: " << queryctrl.name << std::endl;
                std::cout << "  Type: " << queryctrl.type << std::endl;
                std::cout << "  Minimum: " << queryctrl.minimum << std::endl;
                std::cout << "  Maximum: " << queryctrl.maximum << std::endl;
                std::cout << "  Step: " << queryctrl.step << std::endl;
                std::cout << "  Default: " << queryctrl.default_value << std::endl << std::endl;
            }

            queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }

        if (errno != EINVAL) {
            close(file_descriptor);
            std::cerr << "fails for reasons other than reaching the end of the control list" << std::endl;
            return;
        }

    } catch (...) {
        close(file_descriptor);
        throw;
    }

    close(file_descriptor);
}
