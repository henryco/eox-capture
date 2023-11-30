//
// Created by henryco on 11/19/23.
//

#include "texture_1.h"

namespace xogl {

    const std::string Texture1::vertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;

        out vec2 TexCoord;

        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )glsl";

    const std::string Texture1::fragmentShaderSource = R"glsl(
            #version 330 core
            out vec4 FragColor;

            in vec2 TexCoord;
            uniform sampler2D texture_image;

            void main() {
                FragColor = texture(texture_image, TexCoord);
            }
        )glsl";

    unsigned char* Image::getPointer() const {
        return pointer;
    }

    GLsizei Image::getWidth() const {
        return width;
    }

    GLsizei Image::getHeight() const {
        return height;
    }

    GLenum Image::getFormat() const {
        return format;
    }

    Image::Image(unsigned char* pointer,
                 GLsizei width,
                 GLsizei height,
                 GLenum format) :
                 pointer(pointer),
                 width(width),
                 height(height),
                 format(format) {}

    void Texture1::render(Image image) {
        setImage(image);
        render();
    }

    void Texture1::init() {

        // [[Vx, Vy, Vz, Vx, Vy], ...]
        //   0   4   8   12  16
        // V: stride 20 (5*float), offset 0
        // T: stride 20 (5*float), offset 12 (3*float)
        const GLfloat vertices[] = {
                1.0f, 1.0f,0.0f, 1.0f, 0.0f, // top right
                1.0f, -1.0f, 0.0f, 1.0f, 1.0f, // bottom right
                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // bottom left
                -1.0f, 1.0f, 0.0f, 0.0f, 0.0f  // top left
        };

        const GLuint indices[] = {
                0, 1, 3, // first triangle
                1, 2, 3  // second triangle
        };

        { // VAO
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);

            glBindVertexArray(vao);

            { // VBO
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            }

            { // EBO
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
            }

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0); // NOLINT(*-use-nullptr)
            glEnableVertexAttribArray(0);

            // texture cord attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // unbind
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        { // Texture
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        shader.init();
        tex_loc = glGetUniformLocation(shader.getHandle(), "textureSampler");
    }

    void Texture1::setImage(const Image &image) const {
        if (!image.getPointer()) {
            return;
        }

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     image.getWidth(),
                     image.getHeight(),
                     0,
                     image.getFormat(),
                     GL_UNSIGNED_BYTE,
                     image.getPointer());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture1::render() {
        glUseProgram(shader.getHandle());
        glUniform1i(tex_loc, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // NOLINT(*-use-nullptr)

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    void Texture1::cleanup() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteTextures(1, &texture);
    }

    Texture1::~Texture1() {
        cleanup();
    }

    Texture1::Texture1(Texture1 &&ref) noexcept :
    vao(ref.vao),
    vbo(ref.vbo),
    ebo(ref.ebo),
    texture(ref.texture),
    tex_loc(ref.tex_loc),
    shader(std::move(ref.shader)) {}

} // xogl