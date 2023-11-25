//
// Created by henryco on 11/25/23.
//

#ifndef STEREOX_LINUX_VIDEO_H
#define STEREOX_LINUX_VIDEO_H

#include <linux/videodev2.h>
#include <vector>

namespace sex::v4l2 {
    typedef struct v4l2_queryctrl V4L2_QueryCtrl;

    std::vector<V4L2_QueryCtrl> get_camera_props(int id);
}


#endif //STEREOX_LINUX_VIDEO_H
