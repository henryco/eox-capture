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

        unsigned int shaderProgram;

    public:
        SimpleShader(std::string vertex, std::string fragment);
        SimpleShader(SimpleShader&& ref) noexcept;

        ~SimpleShader();

        void init();
        void cleanup() const;

        [[nodiscard]] unsigned int getHandle() const;
    };
}




#endif //STEREOX_SIMPLE_SHADER_H
