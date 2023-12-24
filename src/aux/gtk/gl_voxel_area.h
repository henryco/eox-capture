//
// Created by henryco on 12/24/23.
//

#ifndef STEREOX_GL_VOXEL_AREA_H
#define STEREOX_GL_VOXEL_AREA_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <gtkmm/glarea.h>
#include <gtkmm/box.h>
#include <opencv2/core/mat.hpp>
#include "../ogl/camera/camera.h"
#include "../ogl/render/voxels.h"

namespace eox::xgtk {

    class GLVoxelArea : public Gtk::Box {
        static inline const auto log =
                spdlog::stdout_color_mt("gl_voxel_area");

    private:
        cv::Mat positions;
        cv::Mat colors;
        bool mat = false;

        eox::ogl::Camera camera;
        eox::ogl::Voxels voxels;
        Gtk::GLArea gl_area;

        int width = 0;
        int height = 0;
        int v_w = 0;
        int v_h = 0;

        long total;

    protected:
        bool render_fn(const Glib::RefPtr<Gdk::GLContext> & context);

        void init_fn(bool bgr);

    public:
        void setPoints(const float *pos, const float *color);

        void setPoints(cv::Mat pos, cv::Mat color);

        void setPointSize(float size);

        void init(long total, bool bgr = false, int width = 0, int height = 0);

        void update();

        void scale(float _scale);

        void resize(int width = -1, int height = -1);

        int getViewWidth() const;

        int getViewHeight() const;
    };

} // eox

#endif //STEREOX_GL_VOXEL_AREA_H
