//
// Created by henryco on 11/25/23.
//

#ifndef STEREOX_LINUX_VIDEO_H
#define STEREOX_LINUX_VIDEO_H

#include <linux/videodev2.h>
#include <vector>
#include <ostream>
#include <map>

namespace eox::v4l2 {

    struct v4l2_queryctrl_ext : public v4l2_queryctrl {
        __s32 value;
    };

    typedef struct {
        uint32_t id;
        int32_t value;
    } serial_v4l2_control;

    typedef struct v4l2_queryctrl_ext V4L2_QueryCtrl;
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
    std::vector<eox::v4l2::V4L2_QueryCtrl> get_camera_props(uint device_id);

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
    bool set_camera_prop(uint device_id, V4L2_Control control);

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
     * @see eox::v4l2::V4L2_Control
     */
    std::vector<bool> set_camera_prop(uint device_id, std::vector<V4L2_Control> controls);

    /**
     * @brief Reset the settings of the specified video device to its default values.
     *
     * This function resets the video settings, such as brightness, contrast, saturation, etc., of a specified video device
     * to its default values.
     *
     * @param device_id The ID of the video device to reset the settings of.
     */
    void reset_defaults(uint device_id);

    /**
     * @brief Writes the V4L2 control to the output stream.
     *
     * This function serializes the V4L2 control object into the provided output stream.
     *
     * @param os The output stream to write the control to.
     * @param control The V4L2 control object to be serialized.
     *
     * @note It DOES NOT close nor flush the stream
     */
    void write_control(std::ostream &out, const V4L2_Control &control);

    /**
     * @brief Writes V4L2 control information to an output stream.
     *
     * This function writes the device ID and the size of the control array to the output stream,
     * followed by the control data. Each control's information is serialized and written
     * to the stream. This is typically used for saving or transmitting the state of V4L2 controls.
     *
     * @param os Reference to the output stream where control data will be written.
     * @param device_id The unique identifier for the V4L2 device.
     * @param controls A vector of V4L2_Control objects representing the controls to be written.
     */
    void write_control(std::ostream &os, uint device_id, const std::vector<V4L2_Control> &controls);

    /**
     * @brief Reads control values from the given input stream.
     *
     * This function is responsible for reading control values from the
     * provided input stream. It expects the values to be in a certain
     * format and will extract them accordingly. The format of the input
     * stream should follow the requirements of the V4L2 API.
     *
     * @note This function assumes the input stream is open and can be read.
     *
     * @param is The input stream to read control values from.
     */
    V4L2_Control read_control(std::istream &is);

    /**
     * @brief Reads control values from the input stream.
     *
     * This function reads a specified number of control values from the given input stream.
     *
     * @param is    The input stream from which to read the control values.
     * @param num   The number of control values to read.
     *
     * @note This function assumes the input stream is open and can be read.
     */
    std::vector<V4L2_Control> read_control(std::istream &is, size_t num);

    /**
     * @brief Reads control information for multiple V4L2 devices from an input stream.
     *
     * This function parses a stream of V4L2 control data, organizing it into a map. Each entry in the map
     * corresponds to a V4L2 device identified by its ID. The value for each entry is a vector of V4L2_Control
     * structures representing the control settings for that device.
     *
     * The input stream should have data in a specific format: pairs of headers and control data. Each pair
     * consists of a device ID (as uint) and the number of controls (as size_t), followed by the control data.
     *
     * @param is An input stream (std::istream&) from which V4L2 control data is read. The stream should be in
     *           binary format and must be already opened and in a good state.
     *
     * @return std::map<uint, std::vector<eox::v4l2::V4L2_Control>> - A map where each key is a device ID and
     *         the corresponding value is a vector of V4L2_Control structures for that device. If no data is read,
     *         or if the input stream is not in the expected format, the returned map may be empty.
     *
     * @note The function will continue reading from the stream until EOF is reached. It is assumed that the
     *       stream is well-formed and correctly formatted according to the expected V4L2 control data structure.
     */
    std::map<uint, std::vector<V4L2_Control>> read_controls(std::istream &is);
}


#endif //STEREOX_LINUX_VIDEO_H
