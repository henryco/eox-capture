//
// Created by henryco on 12/14/23.
//

#include <opencv2/imgproc.hpp>
#include "point_cloud.h"

namespace eox::ocv {

    PointCloud::PointCloud(cv::UMat disparities, cv::UMat points, cv::UMat colors)
            :
            disparities(std::move(disparities)),
            points(std::move(points)),
            colors(std::move(colors)) {
    }

    void PointCloud::write_to_ply(std::ostream &out) const {
        const std::string source = R"ocl(
__kernel void merge_points(
    __global const float* positions,
    __global const uchar* colors,
    __global float* output,
    const unsigned int width,
    const unsigned int height
) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x < width && y < height) {
        int idx = y * width + x;

        int outIdx = idx * 6; // 6 columns (x, y, z, r, g, b) per row
        int posIdx = idx * 3; // 3 components (x, y, z) per position
        int colIdx = idx * 3; // 3 components (b, g, r) per color

        // Copy position data
        output[outIdx + 0] = positions[posIdx + 0];
        output[outIdx + 1] = positions[posIdx + 1];
        output[outIdx + 2] = positions[posIdx + 2];

        // Copy color data
        output[outIdx + 3] = colors[colIdx + 2];
        output[outIdx + 4] = colors[colIdx + 1];
        output[outIdx + 5] = colors[colIdx + 0];

        output[3] = 10.f;
    }
}
)ocl";
        eox::ocl::Kernel oclProgram;
        oclProgram.compile(source);
        oclProgram.procedure("merge_points");

        cv::ocl::Kernel kernel = oclProgram.get_kernel();
        cv::UMat merged(points.rows * points.cols, 6, CV_32F, cv::USAGE_ALLOCATE_DEVICE_MEMORY);

        {
            int idx = 0;
            idx = kernel.set(idx, cv::ocl::KernelArg::PtrReadOnly(points));
            idx = kernel.set(idx, cv::ocl::KernelArg::PtrReadOnly(colors));
            idx = kernel.set(idx, cv::ocl::KernelArg::PtrWriteOnly(merged));
            idx = kernel.set(idx, (uint) points.cols);
            kernel.set(idx, (uint) points.rows);
        }

        size_t g_size[2] = {(size_t) points.cols, (size_t) points.rows};
        bool success = kernel.run(2, g_size, nullptr, true);

        if (!success) {
            log->error("opencl kernel error");
            throw std::runtime_error("opencl kernel error");
        }

        cv::Mat result;
        merged.copyTo(result);

        int total = 0;
        std::stringstream stream;
        for (int i = 0; i < result.rows; i++) {
            if (result.at<float>(i, 2) >= 10000.f)
                continue;
            stream << result.at<float>(i, 0) << " ";
            stream << result.at<float>(i, 1) << " ";
            stream << result.at<float>(i, 2) << " ";
            stream << (uint) result.at<float>(i, 3) << " ";
            stream << (uint) result.at<float>(i, 4) << " ";
            stream << (uint) result.at<float>(i, 5) << "\n";
            total++;
        }

        out << "ply\n";
        out << "format ascii 1.0\n";
        out << "element vertex " << total << "\n";
        out << "property float x\n";
        out << "property float y\n";
        out << "property float z\n";
        out << "property uchar red\n";
        out << "property uchar green\n";
        out << "property uchar blue\n";
        out << "end_header\n";
        out << stream.str();
        out.flush();
    }
} // eox