//
// Created by henryco on 12/20/23.
//

#ifndef STEREOX_KERNEL_H
#define STEREOX_KERNEL_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <opencv2/core/ocl.hpp>
#include <map>

namespace eox::ocl {

    class Kernel {
    public:
        static inline const auto log =
                spdlog::stdout_color_mt("cl_kernels");

        std::unique_ptr<cv::ocl::ProgramSource> source;
        std::unique_ptr<cv::ocl::Program> program;
        std::map<std::string, cv::ocl::Kernel> kernels;

        Kernel() = default;

        void compile(const std::string& kernel, const std::string& flags = "");

        cv::ocl::Kernel& procedure(const std::string &name, const std::string &build_opts = "");

        cv::ocl::Kernel &get_kernel(const std::string &name);

        cv::ocl::Kernel &get_kernel();
    };

} // ocl
// eox

#endif //STEREOX_KERNEL_H
