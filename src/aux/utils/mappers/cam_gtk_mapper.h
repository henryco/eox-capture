//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_CAM_GTK_MAPPER_H
#define STEREOX_CAM_GTK_MAPPER_H

#include <vector>
#include "../../gtk/gtk_cam_params.h"
#include "../../ocv/stereo_camera.h"

namespace sex::mappers {

    namespace cam_gtk {

        std::vector<sex::xgtk::GtkCamProp> map(const std::vector<sex::xocv::camera_control> &vec) {
            std::vector<sex::xgtk::GtkCamProp> parameters;
            parameters.reserve(vec.size());
            for (const auto &p: vec) {
                parameters.emplace_back(p.id, p.type, p.name, p.min, p.max, p.step, p.default_value, p.value);
            }
            return parameters;
        }

        std::vector<uint> index(const std::vector<sex::data::camera_properties>& props, uint device_id, bool homogeneous) {
            std::vector<uint> indexes;
            for (const auto &item: props) {
                if (item.id == device_id || homogeneous)
                    indexes.push_back(item.index);
            }
            return indexes;
        }

    }

}

#endif //STEREOX_CAM_GTK_MAPPER_H
