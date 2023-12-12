//
// Created by henryco on 11/21/23.
//

#include <iostream>
#include <utility>
#include "gl_image.h"

namespace sex::xgtk {

    GLImage::~GLImage() {
        textures.clear();
        glAreas.clear();
        frames.clear();
    }

    std::function<bool(const Glib::RefPtr<Gdk::GLContext> &)> GLImage::renderFunc(int num) {
        return [num, this](const Glib::RefPtr<Gdk::GLContext>& context) -> bool {

            if (initialized.empty() || !initialized[num]) {
                log->warn("source [{}] is not initialized yet!", num);
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

    std::function<void()> GLImage::initFunc(size_t num) {
        return [num, this]() {
            auto& area = glAreas[num];
            area->make_current();
            area->throw_if_error();
            textures[num]->init();
            initialized[num] = true;
        };
    }

    void GLImage::update() {
        for (auto &area: glAreas) {
            area->queue_render();
        }
    }

    void GLImage::setFrames(const std::vector<cv::Mat>& _frames) {
        frames.clear();
        for (auto item: _frames) {
            // YES, we NEED TO COPY IT
            frames.push_back(std::move(item));
        }
    }
    void GLImage::update(const std::vector<cv::Mat>& _frames) {
        setFrames(_frames);
        update();
    }

    void GLImage::init(size_t number, int _width, int _height, std::vector<std::string> ids, GLenum _format) {
        init(1, number, number, _width, _height, std::move(ids), _format);
    }

    void GLImage::init(size_t number, int _width, int _height, GLenum _format) {
        init(1, number, number, _width, _height, _format);
    }

    void GLImage::init(size_t _rows, size_t _cols, size_t _number, int _width, int _height, GLenum _format) {
        std::vector<std::string> _ids;
        _ids.reserve(_number);
        for (int i = 0; i < _number; i++) {
            _ids.push_back(std::to_string(i));
        }
        init(_rows, _cols, _number, _width, _height, _ids, _format);
    }

    void GLImage::init(size_t _rows, size_t _cols, size_t _number, int _width, int _height, std::vector<std::string> _ids, GLenum _format) {
        this->width = _width;
        this->height = _height;
        this->format = _format;

        initialized.reserve(_number);
        textures.reserve(_number);
        glAreas.reserve(_number);
        frames.reserve(_number);

        this->v_w = (int) _number * _width;
        this->v_h = _height;

        this->rows = _rows;
        this->cols = _cols;

        auto h_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        for (int i = 0; i < _number; ++i) {
            auto area = std::make_unique<Gtk::GLArea>();
            area->signal_realize().connect(initFunc(i), false);
            area->signal_render().connect(renderFunc(i), false);
            area->set_size_request((int) (v_w / _number), v_h);
            area->set_auto_render(true);

            auto v_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
            auto label = std::make_unique<Gtk::Label>();
            const std::string id = _ids.at(i);
            label->set_label(id);

            v_box->pack_start(*label, Gtk::PACK_SHRINK);
            v_box->pack_start(*area, Gtk::PACK_SHRINK);
            h_box->pack_start(*v_box, Gtk::PACK_SHRINK);

            labels.push_back(std::move(label));
            containers.push_back(std::move(v_box));
            glAreas.push_back(std::move(area));
            textures.push_back(std::make_unique<xogl::Texture1>());
            initialized.push_back(false);
        }

        set_size_request(v_w, v_h);
        set_orientation(Gtk::ORIENTATION_VERTICAL);
        pack_start(*h_box, Gtk::PACK_SHRINK);
        containers.push_back(std::move(h_box));
    }

    void GLImage::scale(float _scale) {
        resize((int) ((float) v_w * _scale), (int) ((float) v_h * _scale));
    }

    void GLImage::resize(int _width, int _height) {
        const auto _number = glAreas.size();

        if (_width == -1 && _height == -1) {
            v_w = (int) _number * width;
            v_h = height;
        }

        else if (_width != -1 && _height == -1) {
            v_w = _width;

            const auto ratio = (float) (width * _number) / (float) height;
            v_h = (int) ((float) v_w / ratio);
        }

        else if (_width == -1) {
            v_h = _height;

            const auto ratio = (float) (width * _number) / (float) height;
            v_w = (int) ((float) v_h * ratio);
        }

        else {
            v_w = _width;
            v_h = _height;
        }

        for (const auto &area: glAreas) {
            area->set_size_request((int) (v_w / _number), v_h);
        }
        set_size_request(v_w, v_h);
    }

    int GLImage::getViewWidth() const {
        return v_w;
    }

    int GLImage::getViewHeight() const {
        return v_h;
    }

} // xgtk