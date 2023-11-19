//
// Created by henryco on 11/19/23.
//

#ifndef STEREOX_TEXTURE_1_H
#define STEREOX_TEXTURE_1_H

#include <GL/gl.h>
#include <GLES3/gl3.h>
#include "../shader/simple_shader.h"

namespace xogl {

    class Image {
    private:
        unsigned char* pointer;
        GLsizei width;
        GLsizei height;
        GLenum format;

    public:
        Image(unsigned char* pointer, GLsizei width, GLsizei height, GLenum format = GL_RGB);
        [[nodiscard]] unsigned char* getPointer() const;
        [[nodiscard]] GLsizei getWidth() const;
        [[nodiscard]] GLsizei getHeight() const;
        [[nodiscard]] GLenum getFormat() const;
    };

    class Texture1 {
    private:
        static const std::string vertexShaderSource;
        static const std::string fragmentShaderSource;
        SimpleShader shader = SimpleShader(vertexShaderSource, fragmentShaderSource);

        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo;
        GLuint texture;
        GLint tex_loc;

    public:
        Texture1() = default;
        ~Texture1();
        void init();
        void render();
        void render(Image image);
        void setImage(const Image &image) const;
        void cleanup();
    };

} // xogl

#endif //STEREOX_TEXTURE_1_H
