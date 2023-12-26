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

        orbit_basis = glm::mat3(basis);

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

        orbit_basis = glm::mat3(basis);

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

        orbit_basis = glm::mat3(basis);

        return *this;
    }

    Camera &Camera::set_basis(glm::vec3 r, glm::vec3 u, glm::vec3 f) {
        basis = glm::mat3(r, u, f);
        orbit_basis = glm::mat3(basis);
        return *this;
    }

    Camera &Camera::set_position(float x, float y, float z) {
        // here we are using absolute (world) coordinate system
        position = glm::vec3(x, y, z);
        return *this;
    }

    Camera &Camera::set_target(float x, float y, float z) {
        // here we are using absolute (world) coordinate system
        target = glm::vec3(x, y, z);
        return *this;
    }

    Camera &Camera::move_free(float x, float y, float z) {
        // here we are using absolute (world) coordinate system
        const auto distance = glm::distance(position, target);

        position = glm::vec3(x, y, z);

        // moving free but also moving target respectively
        target = position + (distance * BASIS_F);

        view_matrix = glm::lookAt(position, target, BASIS_U);
        refresh_basis();

        return *this;
    }

    Camera &Camera::move_lock(float x, float y, float z) {
        // here we are using absolute (world) coordinate system
        position = glm::vec3(x, y, z);
        return look_at(target.x, target.y, target.z);
    }


    Camera &Camera::translate_free(float x, float y, float z) {
        // here we are using absolute (world) coordinate system
        const auto pos = position + glm::vec3(x, y, z);
        return move_free(pos.x, pos.y, pos.z);
    }

    Camera &Camera::translate_lock(float x, float y, float z) {
        // here we are using absolute (world) coordinate system
        const auto pos = position + glm::vec3(x, y, z);
        return move_lock(pos.x, pos.y, pos.z);
    }

    Camera &Camera::translate_free_relative(float r, float u, float f) {
        // inverse to undo rotation
        const auto o_pos = glm::inverse(basis) * position;
        // then add position
        const auto n_pos = o_pos + glm::vec3(r, u, f);
        // then rotate again
        const auto pos = basis * n_pos;
        return move_free(pos.x, pos.y, pos.z);
    }

    Camera &Camera::translate_lock_relative(float r, float u, float f) {
        // inverse to undo rotation
        const auto o_pos = glm::inverse(basis) * position;
        // then add position
        const auto n_pos = o_pos + glm::vec3(r, u, f);
        // then rotate again
        const auto pos = basis * n_pos;
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

        // TODO FIXME, THERE IS AN ERROR

        const auto distance = glm::max(dist, 0.1f);
        const auto elevation = glm::clamp(
                elevation_rad,
                -glm::half_pi<float>() * 0.975f,
                glm::half_pi<float>() * 0.975f
        );

        const auto target_orbit = orbit_basis * target;

        const auto position_orbit = glm::vec3(
                target_orbit.x + distance * glm::cos(elevation) * glm::sin(azimuth_rad),
                target_orbit.y + distance * glm::sin(elevation),
                target_orbit.z + distance * glm::cos(elevation) * glm::cos(azimuth_rad)
        );

        position = glm::inverse(orbit_basis) * position_orbit;

        view_matrix = glm::lookAt(position, target, orbit_basis[1]);
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

    float Camera::get_lock_distance() const {
        // distance is distance
        return glm::distance(position, target);
    }

    float Camera::get_lock_elevation() const {
        // here we are using coordinate system of rotated camera
        const auto py = (orbit_basis * position).y;
        const auto ty = (orbit_basis * target).y;

        // do not forget about sign, because distance is absolute
        const auto sign = py > ty ? 1.f : -1.f;

        // basic trigonometry
        return sign * glm::asin(glm::distance(py, ty) / get_lock_distance());
    }

    float Camera::get_lock_azimuth() const {
        // this gives us coordinates of camera and target in rotated camera coordinate system
        const auto p = orbit_basis * position;
        const auto t = orbit_basis * target;

        // translated to origin of polar coordinate system
        const auto pp = glm::vec2(p.x - t.x, p.z - t.z);

        // angle in polar coordinate system
        return glm::atan(pp.x, pp.y);
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

    const glm::mat3 &Camera::get_orbit_basis() const {
        return orbit_basis;
    }

} // eox
#pragma clang diagnostic pop