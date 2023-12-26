//
// Created by henryco on 12/22/23.
//

#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
namespace eox::ogl {

#define BASIS_R basis[0]
#define BASIS_U basis[1]
#define BASIS_F basis[2]

    Camera::Camera() {
        look_at(0, 0, 1);
    }

    Camera &Camera::pitch(float rad) {

        const glm::mat4 rotation = glm::rotate(glm::mat4(1.f), rad, glm::normalize(BASIS_R));

        const auto homo_f = glm::vec4(BASIS_F, 0.0f);
        const auto forward = glm::normalize(glm::vec3(rotation * homo_f));
        const auto right = glm::normalize(glm::cross(forward, BASIS_U));
        const auto up = glm::normalize(glm::cross(right, forward));

        target = position + forward;

        view_matrix = glm::lookAt(position, target, up);
        refresh_basis();

        return *this;
    }

    Camera &Camera::roll(float rad) {

        const glm::mat4 rotation = glm::rotate(glm::mat4(1.f), rad, glm::normalize(BASIS_F));

        const auto homo_r = glm::vec4(BASIS_R, 0.0f);
        const auto homo_u = glm::vec4(BASIS_U, 0.0f);

        const auto up = glm::normalize(glm::vec3(rotation * homo_u));
        const auto right = glm::normalize(glm::vec3(rotation * homo_r));
        const auto forward = glm::normalize(glm::cross(right, up));

        target = position + forward;

        view_matrix = glm::lookAt(position, target, up);
        refresh_basis();

        return *this;
    }

    Camera &Camera::yaw(float rad) {

        const glm::mat4 rotation = glm::rotate(glm::mat4(1.f), rad, glm::normalize(BASIS_U));

        const auto homo_f = glm::vec4(BASIS_F, 0.0f);
        const auto homo_r = glm::vec4(BASIS_R, 0.0f);

        const auto forward = glm::normalize(glm::vec3(rotation * homo_f));
        const auto right = glm::normalize(glm::vec3(rotation * homo_r));
        const auto up = glm::normalize(glm::cross(right, forward));

        target = position + forward;

        view_matrix = glm::lookAt(position, target, up);
        refresh_basis();

        return *this;
    }

    Camera &Camera::set_basis(glm::vec3 r, glm::vec3 u, glm::vec3 f) {
        basis = glm::mat3(r, u, f);
        return *this;
    }

    Camera &Camera::set_position(float x, float y, float z) {
        position = glm::vec3(x, y, z);
        return *this;
    }

    Camera &Camera::set_target(float x, float y, float z) {
        target = glm::vec3(x, y, z);
        return *this;
    }

    Camera &Camera::move_free(float x, float y, float z) {
        const auto distance = glm::distance(position, target);

        position = glm::vec3(x, y, z);

        // moving free but also moving target respectively
        target = position + (distance * BASIS_F);

        view_matrix = glm::lookAt(position, target, BASIS_U);
        refresh_basis();

        return *this;
    }

    Camera &Camera::move_lock(float x, float y, float z) {
        position = glm::vec3(x, y, z);
        return look_at(target.x, target.y, target.z);
    }


    Camera &Camera::translate_free(float x, float y, float z) {
        const auto pos = position + glm::vec3(x, y, z);
        return move_free(pos.x, pos.y, pos.z);
    }

    Camera &Camera::translate_lock(float x, float y, float z) {
        const auto pos = position + glm::vec3(x, y, z);
        return move_lock(pos.x, pos.y, pos.z);
    }

    Camera &Camera::look_at(float x, float y, float z) {
        target = glm::vec3(x, y, z);
        view_matrix = glm::lookAt(position, target, BASIS_U);
        refresh_basis();

        return *this;
    }

    Camera &Camera::orbit(float azimuth_rad, float elevation_rad, float distance) {
        return set_orbit(
                get_lock_azimuth() + azimuth_rad,
                get_lock_elevation() + elevation_rad,
                get_lock_distance() + distance
        );
    }

    Camera &Camera::set_orbit(float azimuth_rad, float elevation_rad, float dist) {

        const auto distance = glm::max(dist, 0.1f);
        const auto elevation = glm::clamp(
                elevation_rad,
                -glm::pi<float>() / 2 + 0.1f,
                glm::pi<float>() / 2 - 0.1f
        );

        position = glm::vec3(
                target.x + distance * glm::cos(elevation) * glm::sin(azimuth_rad),
                target.y + distance * glm::sin(elevation),
                target.z + distance * glm::cos(elevation) * glm::cos(azimuth_rad)
        );

        view_matrix = glm::lookAt(position, target, BASIS_U);
        refresh_basis();
        return *this;
    }

    Camera &Camera::perspective(float aspect_ratio, float fov, float z_near, float z_far) {
        proj_matrix = glm::perspective(fov, aspect_ratio, z_near, z_far);
        return *this;
    }

    Camera &Camera::orthographic(float width, float height, float z_near, float z_far) {
        proj_matrix = glm::ortho(0.f, width, 0.f, height, z_near, z_far);
        return *this;
    }

    void Camera::refresh_basis() {
        basis = {
                glm::vec3(view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]),    // R
                glm::vec3(view_matrix[0][1], view_matrix[1][1], view_matrix[2][1]),    // U
                glm::vec3(-view_matrix[0][2], -view_matrix[1][2], -view_matrix[2][2])  // F
        };
    }

    const glm::mat3 &Camera::get_basis() const {
        return basis;
    }

    const glm::vec3 &Camera::get_position() const {
        return position;
    }

    const glm::vec3 &Camera::get_target() const {
        return target;
    }

    const glm::mat4 &Camera::get_view_matrix() const {
        return view_matrix;
    }

    const glm::mat4 &Camera::get_projection_matrix() const {
        return proj_matrix;
    }

    float Camera::get_lock_distance() const {
        return glm::distance(position, target);
    }

    float Camera::get_lock_elevation() const {
        return glm::asin(glm::normalize(target - position).y);
    }

    float Camera::get_lock_azimuth() const {
        const auto direction = target - position;
        const auto plane_dir = glm::normalize(glm::vec3(direction.x, 0.f, direction.z));
        return std::atan2(plane_dir.z, plane_dir.x);
    }

} // eox
#pragma clang diagnostic pop