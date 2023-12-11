//
// Created by henryco on 12/11/23.
//

#include "cam_gtk_mapper.h"

std::vector<sex::xgtk::GtkCamProp> sex::mappers::cam_gtk::map(const std::vector<sex::xocv::camera_control> &vec) {
    std::vector<sex::xgtk::GtkCamProp> parameters;
    parameters.reserve(vec.size());
    for (const auto &p: vec) {
        parameters.emplace_back(p.id, p.type, p.name, p.min, p.max, p.step, p.default_value, p.value);
    }
    return parameters;
}

std::vector<uint>
sex::mappers::cam_gtk::index(const std::vector<sex::data::camera_properties> &props, uint device_id, bool homogeneous) {
    std::vector<uint> indexes;
    for (const auto &item: props) {
        if (item.id == device_id || homogeneous)
            indexes.push_back(item.index);
    }
    return indexes;
}
