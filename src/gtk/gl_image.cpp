//
// Created by henryco on 11/21/23.
//

#include <iostream>
#include "gl_image.h"

namespace xgtk {

    GLImage::~GLImage() {
        textures.clear();
        glAreas.clear();
        frames.clear();
    }

    std::function<bool(const Glib::RefPtr<Gdk::GLContext> &)> GLImage::renderFunc(int num) {
        return [num, this](const Glib::RefPtr<Gdk::GLContext>& context) -> bool {

            if (initialized.empty() || !initialized[num]) {
                std::cerr << "GLImage [" << num << "] is not initialized yet!" << std::endl;
                return true;
            }

            auto& texture = textures[num];
            if (texture == nullptr) {
                return false;
            }

            if (frames.empty()) {
                return true;
            }

            glClearColor(.0f, .0f, .0f, .0f);
            glClear(GL_COLOR_BUFFER_BIT);

            texture->render(xogl::Image(frames[num].data, width, height, format));

            return true;
        };
    }

    std::function<void()> GLImage::initFunc(int num) {
        return [num, this]() {
            auto& area = glAreas[num];
            area->make_current();
            area->throw_if_error();
            textures[num]->init();
            initialized[num] = true;
        };
    }

    void GLImage::init(int _number, int _width, int _height, GLenum _format) {
        this->width = _width;
        this->height = _height;
        this->format = _format;

        initialized.reserve(_number);
        textures.reserve(_number);
        glAreas.reserve(_number);
        frames.reserve(_number);

        for (int i = 0; i < _number; ++i) {
            auto area = std::make_unique<Gtk::GLArea>();
            area->signal_realize().connect(initFunc(i), false);
            area->signal_render().connect(renderFunc(i), false);
            area->set_size_request(_width, _height);
            area->set_auto_render(true);

            h_box.pack_end(*area, Gtk::PACK_SHRINK);
            glAreas.push_back(std::move(area));
            textures.push_back(std::make_unique<xogl::Texture1>());
            initialized.push_back(false);
        }

        set_size_request(_number * _width, height);
        set_orientation(Gtk::ORIENTATION_VERTICAL);
        pack_end(h_box, Gtk::PACK_SHRINK);
    }

    void GLImage::update() {
        for (auto &area: glAreas) {
            area->queue_render();
        }
    }

    void GLImage::setFrames(std::vector<cv::Mat> _frames) { // NOLINT(*-unnecessary-value-param)
        frames.clear();
        for (auto item: _frames) {
            frames.push_back(std::move(item));
        }
    }

    void GLImage::update(std::vector<cv::Mat> _frames) {
        setFrames(std::move(_frames));
        update();
    }

} // xgtk