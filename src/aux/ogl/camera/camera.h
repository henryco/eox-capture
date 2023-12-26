//
// Created by henryco on 12/22/23.
//

#ifndef STEREOX_CAMERA_H
#define STEREOX_CAMERA_H

#include <glm/mat4x4.hpp>

namespace eox::ogl {

    class Camera {
    private:
        glm::mat3 basis = glm::mat3(
                glm::vec3(1.f, 0.f, 0.f), // R
                glm::vec3(0.f, 1.f, 0.f), // U
                glm::vec3(0.f, 0.f, 1.f)  // F
        );

        glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 target = glm::vec3(0.f, 0.f, 1.f);
        glm::mat4 view_matrix = glm::mat4(1.f);
        glm::mat4 proj_matrix = glm::mat4(1.f);

    public:

        Camera();

        Camera &set_basis(glm::vec3 r, glm::vec3 u, glm::vec3 f);

        Camera &set_position(float x, float y, float z);

        Camera &set_target(float x, float y, float z);

        Camera &move_free(float x, float y, float z);

        Camera &move_lock(float x, float y, float z);

        Camera &translate_free(float x, float y, float z);

        Camera &translate_lock(float x, float y, float z);

        Camera &pitch(float rad);

        Camera &roll(float rad);

        Camera &yaw(float rad);

        Camera &look_at(float x, float y, float z);

        Camera &orbit(float azimuth_rad_d, float elevation_rad_d, float distance_d);

        Camera &set_orbit(float azimuth_rad, float elevation_rad, float distance);

        Camera &perspective(float aspect_ratio, float fov, float z_near, float z_far);

        Camera &orthographic(float width, float height, float z_near, float z_far);

        [[nodiscard]] const glm::mat3 &get_basis() const;

        [[nodiscard]] const glm::vec3 &get_position() const;

        [[nodiscard]] const glm::vec3 &get_target() const;

        [[nodiscard]] const glm::mat4 &get_view_matrix() const;

        [[nodiscard]] const glm::mat4 &get_projection_matrix() const;

        [[nodiscard]] float get_lock_distance() const;

        [[nodiscard]] float get_lock_elevation() const;

        [[nodiscard]] float get_lock_azimuth() const;

    protected:
        void refresh_basis();
    };

} // eox

#endif //STEREOX_CAMERA_H