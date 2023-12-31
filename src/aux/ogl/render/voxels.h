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

        long total = 0;
        float size = 200.;

        // RGBA
        float clear_color[4] = {
                .0f,
                .274f,
                .44f,
                .1f
        };

    public:
        Voxels() = default;

        ~Voxels();

        void init(long count, bool bgr = false);

        void render(const float **view_mat, const float **projection_mat);

        void render(const float *view_mat, const float *projection_mat);

        void render(const float (view_mat)[4][4], const float (projection_mat)[4][4]);

        Voxels &clear();

        Voxels &setPoints(const void *pos, const void *color);

        Voxels &setPointSize(float size);

        Voxels &setClearColor(float r, float g, float b, float a = 0.f);

        void cleanup();

    protected:
        void renderFlatten(const float *mvp, const float *proj);
    };

} // eox

#endif //STEREOX_VOXELS_H
