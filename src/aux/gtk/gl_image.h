//
// Created by henryco on 11/21/23.
//

#ifndef STEREOX_GL_IMAGE_H
#define STEREOX_GL_IMAGE_H

#include <gtkmm/box.h>
#include <gtkmm/glarea.h>
#include <opencv2/core/mat.hpp>
#include "../ogl/render/texture_1.h"
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace sex::xgtk {

class GLImage : public Gtk::Box {
private:

    static inline const auto log =
            spdlog::stdout_color_mt("gl_image");

    Gtk::Box h_box = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);

    std::vector<std::unique_ptr<xogl::Texture1>> textures;
    std::vector<std::unique_ptr<Gtk::GLArea>> glAreas;
    std::vector<cv::Mat> frames;
    std::vector<bool> initialized;

    GLenum format = GL_RGB;
    int width = 0;
    int height = 0;

protected:
    std::function<bool(const Glib::RefPtr<Gdk::GLContext> &)> renderFunc(int num);
    std::function<void()> initFunc(size_t num);

public:
    GLImage() = default;
    ~GLImage() override;

    void init(size_t number, int width, int height, GLenum format = GL_RGB);
    void setFrames(const std::vector<cv::Mat>& _frames);
    void update(const std::vector<cv::Mat>& _frames);
    void update();

};

} // xgtk

#endif //STEREOX_GL_IMAGE_H
