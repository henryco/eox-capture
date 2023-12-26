//
// Created by henryco on 12/24/23.
//

#include "gl_voxel_area.h"
#include "glm/trigonometric.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cmath>
#include <gtkmm/eventbox.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
namespace eox::xgtk {

    void GLVoxelArea::init(long _total, bool bgr, int _width, int _height) {
        total = _total;

        width = _width;
        height = _height;
        v_w = _width;
        v_h = _height;


        camera.perspective((float) width / (float) height, glm::radians(70.f), 0.1, 1000);

        camera.roll(glm::radians(180.f));
        camera.set_position(0, 0, -10);
        camera.look_at(0, 0, 100);

        gl_area.signal_realize().connect([this, bgr]() {
            init_fn(bgr);
        });
        gl_area.signal_render().connect([this](const Glib::RefPtr<Gdk::GLContext> &c) {
            return render_fn(c);
        });

        gl_area.set_size_request(v_w, v_h);
        gl_area.set_auto_render(true);

        auto event_box = Gtk::make_managed<Gtk::EventBox>();
        event_box->add(gl_area);

        event_box->add_events(
                Gdk::SCROLL_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::POINTER_MOTION_MASK |
                Gdk::BUTTON_RELEASE_MASK);

        event_box->signal_scroll_event().connect([this](GdkEventScroll *e) {

            const float step = 5;

            float direction = 1;
            if (e->direction == GDK_SCROLL_UP) {
                direction = -1;
            } else if (e->direction != GDK_SCROLL_DOWN) {
                return true;
            }

            log->info("D: {}, A: {}, E: {}",
                      camera.get_lock_distance(),
                      glm::degrees(camera.get_lock_azimuth()),
                      glm::degrees(camera.get_lock_elevation()));

            camera.orbit(0.f, 0.f, 1.f * step * direction);
            return true;
        });

        event_box->signal_button_press_event().connect([this](GdkEventButton *e) {
            mouse_l_r[0] = false;
            mouse_l_r[1] = false;
            if (e->button == 1)
                mouse_l_r[0] = true;
            if (e->button == 2)
                mouse_l_r[1] = true;
            return false;
        });

        event_box->signal_button_release_event().connect([this](GdkEventButton *e) {
            mouse_l_r[0] = false;
            mouse_l_r[1] = false;
            return false;
        });

        event_box->signal_motion_notify_event().connect([this](GdkEventMotion *e) {

            const float dx = 1.f * (e->x - mouse_pos[0]);
            const float dy = 1.f * (e->y - mouse_pos[1]);

            if (mouse_l_r[0]) {
                camera.orbit(glm::radians(0.5f * dx), glm::radians(0.5f * dy), 0.f);
            } else if (mouse_l_r[1]) {
                camera.translate_free(-0.5 * dx, -0.5 * dy, 0);
            }

            mouse_pos[0] = (float) e->x;
            mouse_pos[1] = (float) e->y;
            return true;
        });

        auto h_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        h_box->set_halign(Gtk::ALIGN_CENTER);
        h_box->pack_start(*event_box, Gtk::PACK_SHRINK);

        pack_start(*h_box, Gtk::PACK_SHRINK);

        set_size_request(v_w, v_h);
        set_orientation(Gtk::ORIENTATION_VERTICAL);
        set_valign(Gtk::ALIGN_CENTER);
    }

    void GLVoxelArea::init_fn(bool bgr) {
        gl_area.make_current();
        gl_area.throw_if_error();
        voxels.init(total, bgr);
    }

    bool GLVoxelArea::render_fn(const Glib::RefPtr<Gdk::GLContext> &_) {
        if (mat) {
            voxels.setPoints(positions.data, colors.data);
        }

        voxels.render(
                glm::value_ptr(camera.get_view_matrix()),
                glm::value_ptr(camera.get_projection_matrix())
        );

        return true;
    }

    void GLVoxelArea::update() {
        gl_area.queue_render();
    }

    void GLVoxelArea::setPoints(const float *pos, const float *color) {
        voxels.setPoints(pos, color);
        mat = false;
    }

    void GLVoxelArea::setPoints(cv::Mat pos, cv::Mat color) {
        positions = std::move(pos);
        colors = std::move(color);
        mat = true;
    }

    void GLVoxelArea::setPointSize(float size) {
        voxels.setPointSize(size);
    }

    void GLVoxelArea::scale(float _scale) {
        resize((int) ((float) v_w * _scale), (int) ((float) v_h * _scale));
    }

    void GLVoxelArea::resize(int _width, int _height) {
        if (_width == -1 && _height == -1) {
            v_w = width;
            v_h = height;
        } else if (_width != -1 && _height == -1) {
            v_w = _width;

            const auto ratio = (float) width / (float) height;
            v_h = (int) ((float) v_w / ratio);
        } else if (_width == -1) {
            v_h = _height;

            const auto ratio = (float) width / (float) height;
            v_w = (int) ((float) v_h * ratio);
        } else {
            v_w = _width;
            v_h = _height;
        }

        gl_area.set_size_request(v_w, v_h);
        set_size_request(v_w, v_h);
    }

    int GLVoxelArea::getViewWidth() const {
        return v_w;
    }

    int GLVoxelArea::getViewHeight() const {
        return v_h;
    }

} // eox
#pragma clang diagnostic pop