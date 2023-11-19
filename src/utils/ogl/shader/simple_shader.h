//
// Created by henryco on 11/19/23.
//

#ifndef STEREOX_SIMPLE_SHADER_H
#define STEREOX_SIMPLE_SHADER_H


#include <string>
#include <GLES3/gl3.h>

namespace xogl {
    class SimpleShader {
    private:
        std::string vertex;
        std::string fragment;

        GLuint shaderProgram;

    public:
        SimpleShader(std::string vertex, std::string fragment);
        ~SimpleShader();

        void init();
        void cleanup() const;

        [[nodiscard]] GLuint getHandle() const;
    };
}




#endif //STEREOX_SIMPLE_SHADER_H
