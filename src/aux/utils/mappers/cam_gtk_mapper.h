//
// Created by henryco on 11/29/23.
//

#ifndef STEREOX_CAM_GTK_MAPPER_H
#define STEREOX_CAM_GTK_MAPPER_H

#include <vector>
#include "../../gtk/gtk_cam_params.h"
#include "../../ocv/stereo_camera.h"

namespace eox::mappers {

    namespace cam_gtk {

        std::vector<eox::xgtk::GtkCamProp> map(const std::vector<eox::xocv::camera_control> &vec);

        std::vector<uint> index(const std::vector<eox::data::camera_properties>& props, uint device_id, bool homogeneous);

    }

}

#endif //STEREOX_CAM_GTK_MAPPER_H
