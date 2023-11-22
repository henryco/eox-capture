//
// Created by henryco on 11/21/23.
//

#ifndef STEREOX_GL_IMAGE_H
#define STEREOX_GL_IMAGE_H

#include <gtkmm/box.h>
#include <gtkmm/glarea.h>
#include <opencv2/core/mat.hpp>
#include "../ogl/render/texture_1.h"

namespace xgtk {

class GLImage : public Gtk::Box {
private:
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
    std::function<void()> initFunc(int num);

public:
    GLImage() = default;
    ~GLImage() override;

    void init(int number, int width, int height, GLenum format = GL_RGB);
    void setFrames(std::vector<cv::Mat> _frames);
    void update(std::vector<cv::Mat> _frames);
    void update();

};

} // xgtk

#endif //STEREOX_GL_IMAGE_H
