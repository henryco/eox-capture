//
// Created by henryco on 12/21/23.
//

#ifndef STEREOX_VOXELS_H
#define STEREOX_VOXELS_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../shader/simple_shader.h"

namespace eox::ogl {

    class Voxels {
        static inline const auto log =
                spdlog::stdout_color_mt("voxels");

    private:
        xogl::SimpleShader shader = xogl::SimpleShader("TODO", "TODO");

    public:
        Voxels() = default;

        ~Voxels();

        void init();

        void render();

        void cleanup();
    };

} // eox

#endif //STEREOX_VOXELS_H
