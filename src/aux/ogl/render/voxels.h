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
        xogl::SimpleShader shader = xogl::SimpleShader(
                vertex_source,
                fragment_source);

        static const std::string vertex_source;
        static const std::string fragment_source;

        GLuint vao;
        GLuint vbo[2];
        GLint uni_loc[3];

        size_t total = 0;
        float size = 10.;

    public:
        Voxels() = default;

        ~Voxels();

        void init();

        void render(const float **view_mat, const float **projection_mat);

        Voxels &setPoints(const float *pos, const float *color, size_t elements);

        Voxels &setPointSize(float size);

        void cleanup();
    };

} // eox

#endif //STEREOX_VOXELS_H
