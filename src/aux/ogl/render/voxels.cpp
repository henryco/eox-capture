//
// Created by henryco on 12/21/23.
//

#include "voxels.h"

namespace eox::ogl {

    void multiply_4x4(const float **a, const float **b, float result[4][4]) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result[i][j] += a[i][k] * b[k][j];
                }
            }
        }
    }

    void multiply_4x4(const float a[4][4], const float b[4][4], float result[4][4]) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result[i][j] += a[i][k] * b[k][j];
                }
            }
        }
    }

    const std::string Voxels::vertex_source = R"glsl(

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 mvp;
uniform mat4 projection;
uniform float point_size;

out vec4 point_color;

void main() {
    if (aPos.z >= 10000.) {
        gl_Position = vec4(0., 0., 1000., 1.);
        return;
    }

    gl_Position = mvp * vec4(aPos, 1.0);
    point_color = vec4(aColor, 1.0);

    float projectedPointSize = (projection[1][1] * point_size) / gl_Position.w;
    gl_PointSize = max(projectedPointSize, 1.0);
}

)glsl";

    const std::string Voxels::fragment_source = R"glsl(

#version 330 core

in vec4 point_color;
out vec4 FragColor;

void main() {
    FragColor = point_color;
}

)glsl";

    const std::string fragment_source_bgr = R"glsl(

#version 330 core

in vec4 point_color;
out vec4 FragColor;

void main() {
    FragColor = vec4(point_color.b, point_color.g, point_color.r, 1.0);
}

)glsl";

    void Voxels::init(bool bgr) {
        if (bgr) {
            shader = xogl::SimpleShader(
                    vertex_source,
                    fragment_source_bgr);
        }

        glGenBuffers(2, vbo);
        glGenVertexArrays(1, &vao);

        // binding VAO
        glBindVertexArray(vao);

        // [X,Y,Z] | 3x4 bytes
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0); // NOLINT(*-use-nullptr)

        // [R,G,B] | 3x1 bytes
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3 * sizeof(u_char), (void *) 0); // NOLINT(*-use-nullptr)

        // unbind buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        shader.init();

        uni_loc[0] = glGetUniformLocation(shader.getHandle(), "mvp");
        uni_loc[1] = glGetUniformLocation(shader.getHandle(), "projection");
        uni_loc[2] = glGetUniformLocation(shader.getHandle(), "point_size");
    }

    void Voxels::render(const float view_mat[4][4], const float projection_mat[4][4]) {
        float _mvp[4][4];
        multiply_4x4(view_mat, projection_mat, _mvp);

        // reverse row-major to column-major order
        float mvp[16], proj[16];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                const int k = i * 4 + j;
                mvp[k] = _mvp[j][i];
                proj[k] = projection_mat[j][i];
            }
        }

        renderFlatten(mvp, proj);
    }

    void Voxels::render(const float **view_mat, const float **projection_mat) {
        float _mvp[4][4];
        multiply_4x4(view_mat, projection_mat, _mvp);

        // reverse row-major to column-major order
        float mvp[16], proj[16];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                const int k = i * 4 + j;
                mvp[k] = _mvp[j][i];
                proj[k] = projection_mat[j][i];
            }
        }

        renderFlatten(mvp, proj);
    }

    void Voxels::renderFlatten(const float *mvp, const float *proj) {
        glUseProgram(shader.getHandle());

        glUniformMatrix4fv(uni_loc[0], 1, GL_FALSE, mvp);
        glUniformMatrix4fv(uni_loc[1], 1, GL_FALSE, proj);
        glUniform1f(uni_loc[2], size);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, (int) total);
        glBindVertexArray(0);

        glUseProgram(0);
    }

    Voxels &Voxels::setPoints(const void *pos, const void *color, size_t elements) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * (long) total * (long) sizeof(float), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * (long) elements * (long) sizeof(float), pos);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 3 * (long) total * (long) sizeof(u_char), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * (long) elements * (long) sizeof(u_char), color);

        total = elements;
        return *this;
    }

    Voxels &Voxels::setPointSize(float _size) {
        size = _size;
        return *this;
    }

    void Voxels::cleanup() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(2, vbo);
    }

    Voxels::~Voxels() {
        cleanup();
    }

} // eox