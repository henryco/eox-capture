//
// Created by henryco on 12/22/23.
//

#ifndef STEREOX_CAMERA_H
#define STEREOX_CAMERA_H

namespace eox::ogl {

    class Camera {
    public:

        float target[3] = {
                0,
                0,
                0
        };

        float position[3] = {
                0, // x
                0, // y
                0  // z
        };

        float base[3][3] = {
                {1, 0, 0}, // R
                {0, 1, 0}, // U
                {0, 0, 1}, // F
        };

        float view[4][4] = {
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1},
        };

        float projection[4][4] = {
                {},
                {},
                {},
                {},
        };

        Camera& translate_free(float x, float y, float z);

        Camera& translate_lock(float x, float y, float z);

        Camera& pitch(float rad);

        Camera& roll(float rad);

        Camera& yaw(float rad);

        Camera& look_at(float x, float y, float z);

    protected:
        void apply_transform_basis(const float t[3][3]);

        void recalculate_view();

    private:
        static void cross_v3(const float a[3], const float b[3], float* result);

        static float len_v(const float* v, int size);
    };

} // eox

#endif //STEREOX_CAMERA_H