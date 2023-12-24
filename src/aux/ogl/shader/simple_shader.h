//
// Created by henryco on 11/19/23.
//

#ifndef STEREOX_SIMPLE_SHADER_H
#define STEREOX_SIMPLE_SHADER_H


#include <string>
#include <GLES3/gl3.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace xogl {
    class SimpleShader {
    private:
        std::string vertex;
        std::string fragment;
        unsigned int shaderProgram;

    public:
        static inline const auto log =
                spdlog::stdout_color_mt("simple_shader");

        SimpleShader(std::string vertex, std::string fragment);

        SimpleShader(SimpleShader &&ref) noexcept;

        SimpleShader(const SimpleShader &src) = default;

        SimpleShader& operator=(const SimpleShader &src) = default;

        SimpleShader& operator=(SimpleShader &&ref) noexcept;

        ~SimpleShader();

        void init();

        void cleanup() const;

        [[nodiscard]] unsigned int getHandle() const;
    };
}


#endif //STEREOX_SIMPLE_SHADER_H
