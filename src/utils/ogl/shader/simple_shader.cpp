//
// Created by henryco on 11/19/23.
//

#include "simple_shader.h"

#include <utility>
#include <iostream>

namespace xogl {

    GLuint loadAndCompileShader(GLenum type, const char* shaderSrc) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &shaderSrc, NULL);
        glCompileShader(shader);

        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "Shader compilation failed\n" << infoLog << std::endl;
        }

        return shader;
    }

    SimpleShader::SimpleShader(
            std::string vertex,
            std::string fragment):
            vertex(std::move(vertex)),
            fragment(std::move(fragment)) {}

    SimpleShader::~SimpleShader() {
        cleanup();
    }

    void SimpleShader::init() {
        GLuint shaderVertex = loadAndCompileShader(GL_VERTEX_SHADER, vertex.c_str());
        GLuint shaderFragment = loadAndCompileShader(GL_FRAGMENT_SHADER, fragment.c_str());

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, shaderVertex);
        glAttachShader(shaderProgram, shaderFragment);
        glLinkProgram(shaderProgram);

        GLint success;
        GLchar infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "Shader program linking failed\n" << infoLog << std::endl;
        }

        glDeleteShader(shaderVertex);
        glDeleteShader(shaderFragment);
    }

    void SimpleShader::cleanup() const {
        glDeleteProgram(shaderProgram);
    }

    GLuint SimpleShader::getHandle() const {
        return shaderProgram;
    }
}
