//
// Created by henryco on 11/19/23.
//

#include "simple_shader.h"

#include <utility>
#include <iostream>

namespace xogl {

    GLuint loadAndCompileShader(GLenum type, const char *shaderSrc) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &shaderSrc, NULL);
        glCompileShader(shader);

        GLint success = -1;
        GLchar infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            SimpleShader::log->error("Shader compilation failed, {}", infoLog);
        }

        return shader;
    }

    SimpleShader::SimpleShader(
            std::string vertex,
            std::string fragment) :
            vertex(std::move(vertex)),
            fragment(std::move(fragment)),
            shaderProgram(0) {}

    SimpleShader::SimpleShader(SimpleShader &&ref) noexcept:
            vertex(std::move(ref.vertex)),
            fragment(std::move(ref.fragment)),
            shaderProgram(ref.shaderProgram) {}

    SimpleShader &SimpleShader::operator=(SimpleShader &&ref) noexcept {
        vertex = std::move(ref.vertex);
        fragment = std::move(ref.fragment);
        shaderProgram = ref.shaderProgram;
        return *this;
    }

    void SimpleShader::init() {
        GLuint shaderVertex = loadAndCompileShader(GL_VERTEX_SHADER, vertex.c_str());
        GLuint shaderFragment = loadAndCompileShader(GL_FRAGMENT_SHADER, fragment.c_str());

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, shaderVertex);
        glAttachShader(shaderProgram, shaderFragment);
        glLinkProgram(shaderProgram);

        GLint success = -1;
        GLchar infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            log->error("shader program linking failed: {}", infoLog);
        }

        glDeleteShader(shaderVertex);
        glDeleteShader(shaderFragment);
    }

    void SimpleShader::cleanup() const {
        glDeleteProgram(shaderProgram);
    }

    unsigned int SimpleShader::getHandle() const {
        return shaderProgram;
    }

    SimpleShader::~SimpleShader() {
        cleanup();
    }
}
