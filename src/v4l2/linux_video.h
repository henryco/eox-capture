//
// Created by henryco on 11/25/23.
//

#ifndef STEREOX_LINUX_VIDEO_H
#define STEREOX_LINUX_VIDEO_H

#include <linux/videodev2.h>
#include <vector>

namespace sex::v4l2 {

    typedef struct v4l2_queryctrl V4L2_QueryCtrl;
    typedef struct v4l2_control V4L2_Control;

    /**
     * @brief Retrieves camera properties for a given camera ID.
     *
     * This function retrieves the video4linux2 (v4l2) camera properties for a specified camera ID.
     *
     * @param id The ID of the camera.
     * @return The camera properties in a structure.
     *
     * @note This function requires the v4l2 library to be installed.
     * In general it is already installed in most of linux distributions.
     */

    std::vector<sex::v4l2::V4L2_QueryCtrl> get_camera_props(uint device_id);

    /**
     * @brief Sets the camera property for a given device.
     *
     * This function allows setting the camera property identified by `prop_id` to the specified `prop_value`
     * for the camera device identified by `device_id`.
     *
     * @param device_id   The ID of the camera device.
     * @param prop_id     The ID of the camera property to set.
     * @param prop_value  The value to set for the camera property.
     *
     * @return True if the camera property was set successfully, false otherwise.
     */

    bool set_camera_prop(uint device_id, uint prop_id, int prop_value);

    /**
     * @brief Sets the camera property for a given device using v4l2 library.
     *
     * @param device_id The ID of the camera device.
     * @param control The v4l2 control to be set.
     *
     * @return True if the camera property was successfully set, false otherwise.
     */

    bool set_camera_prop(uint device_id, sex::v4l2::V4L2_Control control);

    /**
     * @brief Sets the camera properties for the specified device.
     *
     * This function allows you to configure multiple camera properties at once
     * by providing a list of `V4L2_Control` objects.
     *
     * @param device_id The ID of the camera device.
     * @param controls A vector of `V4L2_Control` objects representing the camera properties to be set.
     * @return Vector of booleans representing results of setting each of properties.
     *
     * @see sex::v4l2::V4L2_Control
     */

    std::vector<bool> set_camera_prop(uint device_id, std::vector<sex::v4l2::V4L2_Control> controls);

    /**
     * @brief Reset the settings of the specified video device to its default values.
     *
     * This function resets the video settings, such as brightness, contrast, saturation, etc., of a specified video device
     * to its default values.
     *
     * @param device_id The ID of the video device to reset the settings of.
     */

    void reset_defaults(uint device_id);
}


#endif //STEREOX_LINUX_VIDEO_H
