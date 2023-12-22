//
// Created by henryco on 12/22/23.
//

#include "camera.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
namespace eox::ogl {

#define R_x base[0][0]
#define R_y base[0][1]
#define R_z base[0][2]

#define U_x base[1][0]
#define U_y base[1][1]
#define U_z base[1][2]

#define F_x base[2][0]
#define F_y base[2][1]
#define F_z base[2][2]

#define T_x target[0]
#define T_y target[1]
#define T_z target[2]

#define P_x position[0]
#define P_y position[1]
#define P_z position[2]

    Camera &Camera::translate_free(float x, float y, float z) {
        P_x = x;
        P_y = y;
        P_z = z;
        recalculate();
        return *this;
    }

    Camera &Camera::translate_lock(float x, float y, float z) {
        P_x = x;
        P_y = y;
        P_z = z;
        return look_at(T_x, T_y, T_z);
    }

    Camera &Camera::pitch(float rad) {
        const float t[3][3] = {
                {1, 0, 0},
                {0, std::cos(rad), -(std::sin(rad))},
                {0, std::sin(rad), std::cos(rad)}
        };
        apply_transform_basis(t);
        return *this;
    }

    Camera &Camera::roll(float rad) {
        const float t[3][3] = {
                {std::cos(rad), 0, std::sin(rad)},
                {0, 1, 0},
                {-(std::sin(rad), 0, std::cos(rad))}
        };
        apply_transform_basis(t);
        return *this;
    }

    Camera &Camera::yaw(float rad) {
        const float t[3][3] = {
                {std::cos(rad),-(std::sin(rad)), 0},
                {std::sin(rad), std::cos(rad), 0},
                {0, 0, 1}
        };
        apply_transform_basis(t);
        return *this;
    }

    Camera &Camera::look_at(float x, float y, float z) {
        T_x = x;
        T_y = y;
        T_z = z;

        const float lx = P_x - x;
        const float ly = P_y - y;
        const float lz = P_z - z;
        const float ld = std::sqrt(std::pow(lx, 2.f) + std::pow(ly, 2.f) + std::pow(lz, 2.f));

        F_x = lx / ld;
        F_y = ly / ld;
        F_z = lz / ld;

        float u_v[3];
        cross_v3(base[1], base[2], u_v);
        float u_d = len_v(u_v, 3);

        R_x = u_v[0] / u_d;
        R_y = u_v[1] / u_d;
        R_z = u_v[2] / u_d;

        float c_v[3];
        cross_v3(base[2], base[0], c_v);
        float c_d = len_v(c_v, 3);

        U_x = c_v[0] / c_d;
        U_y = c_v[1] / c_d;
        U_z = c_v[2] / c_d;

        recalculate();
        return *this;
    }

    void Camera::apply_transform_basis(const float (*t)[3]) {
        const float r_x = t[0][0] * R_x + t[0][1] * R_y + t[0][2] * R_z;
        const float r_y = t[1][0] * R_x + t[1][1] * R_y + t[1][2] * R_z;
        const float r_z = t[2][0] * R_x + t[2][1] * R_y + t[2][2] * R_z;

        const float u_x = t[0][0] * U_x + t[0][1] * U_y + t[0][2] * U_z;
        const float u_y = t[1][0] * U_x + t[1][1] * U_y + t[1][2] * U_z;
        const float u_z = t[2][0] * U_x + t[2][1] * U_y + t[2][2] * U_z;

        const float f_x = t[0][0] * F_x + t[0][1] * F_y + t[0][2] * F_z;
        const float f_y = t[1][0] * F_x + t[1][1] * F_y + t[1][2] * F_z;
        const float f_z = t[2][0] * F_x + t[2][1] * F_y + t[2][2] * F_z;

        const float r_d = std::sqrt(std::pow(r_x, 2.f) + std::pow(r_y, 2.f) + std::pow(r_z, 2.f));
        const float u_d = std::sqrt(std::pow(u_x, 2.f) + std::pow(u_y, 2.f) + std::pow(u_z, 2.f));
        const float f_d = std::sqrt(std::pow(f_x, 2.f) + std::pow(f_y, 2.f) + std::pow(f_z, 2.f));

        R_x = r_x / r_d;
        R_y = r_y / r_d;
        R_z = r_z / r_d;

        U_x = u_x / u_d;
        U_y = u_y / u_d;
        U_z = u_z / u_d;

        F_x = f_x / f_d;
        F_y = f_y / f_d;
        F_z = f_z / f_d;
    }

    void Camera::recalculate() {
        matrix[0][0] = R_x;
        matrix[0][1] = R_y;
        matrix[0][2] = R_z;

        matrix[1][0] = U_x;
        matrix[1][1] = U_y;
        matrix[1][2] = U_z;

        matrix[2][0] = F_x;
        matrix[2][1] = F_y;
        matrix[2][2] = F_z;

        matrix[0][3] = (-1.f) * ((R_x * P_x) + (R_y * P_y) + (R_z * P_z));
        matrix[1][3] = (-1.f) * ((U_x * P_x) + (U_y * P_y) + (U_z * P_z));
        matrix[2][3] = (-1.f) * ((F_x * P_x) + (F_y * P_y) + (F_z * P_z));

        matrix[3][0] = 0;
        matrix[3][1] = 0;
        matrix[3][2] = 0;
        matrix[3][3] = 1;
    }

    cv::Mat Camera::get_cv_mat() {
        return {4, 4, CV_32F, matrix};
    }

    void Camera::cross_v3(const float *a, const float *b, float *result) {
        result[0] = (a[1] * b[2]) - (a[2] * b[1]);
        result[1] = (a[2] * b[0]) - (a[0] * b[2]);
        result[2] = (a[0] * b[1]) - (a[1] * b[0]);
    }

    float Camera::len_v(const float *v, int size) {
        float a = 0;
        for (int i = 0; i < size; i++) {
            a += std::pow(v[i], 2.f);
        }
        return std::sqrt(a);
    }
} // eox
#pragma clang diagnostic pop