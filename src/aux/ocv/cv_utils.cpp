//
// Created by henryco on 12/3/23.
//

#include "cv_utils.h"

#include <opencv2/imgproc.hpp>

namespace eox::ocv {

    cv::Mat img_copy(const cv::Mat &image, int color_space_conv_type, int matrix_data_type) {
        cv::Mat output;
        image.convertTo(output, matrix_data_type);
        return img_copy(output, color_space_conv_type);
    }

    cv::Mat img_copy(const cv::Mat &image, int color_space_conv_type) {
        cv::Mat output;
        cv::cvtColor(image, output, color_space_conv_type);
        return std::move(output);
    }

    cv::Mat img_copy(const cv::Mat &image) {
        cv::Mat output;
        image.copyTo(output);
        return std::move(output);
    }

    Squares find_squares(const cv::Mat &image, int columns, int rows, int flag) {
        cv::Mat gray = img_copy(image, cv::COLOR_BGR2GRAY);
        cv::Mat copy = img_copy(image);

        const auto size = cv::Size(columns - 1, rows - 1);
        const int flags = flag
                          | cv::CALIB_CB_FAST_CHECK
                          | cv::CALIB_CB_NORMALIZE_IMAGE
                          | cv::CALIB_CB_FILTER_QUADS
                          | cv::CALIB_CB_ADAPTIVE_THRESH
        ;

        std::vector<cv::Point2f> corners;
        const bool found = cv::findChessboardCorners(gray, size, corners, flags);

        cv::drawChessboardCorners(copy, size, corners, found);
        return {
                .corners = std::move(corners),
                .original = image,
                .result = std::move(copy),
                .found = found
        };
    }

    CalibrationSolo
    calibrate_solo(std::vector<std::vector<cv::Point2f>> &corners, int width, int height, int rows, int cols) {
        // Prepare object points (0,0,0), (1,0,0), (2,0,0) ... (8,5,0)
        std::vector<cv::Point3f> obj_p;
        for(int i = 0; i < rows - 1; ++i) {
            for(int j = 0; j < cols - 1; ++j) {
                obj_p.emplace_back((float) j, (float) i, 0.0f);
            }
        }

        // Replicate objP for each image
        std::vector<std::vector<cv::Point3f>> object_points;
        object_points.reserve(corners.size());
        for (int i = 0; i < corners.size(); ++i) {
            object_points.push_back(obj_p);
        }

        // output parameters
        cv::Mat camera_matrix, distortion_coefficients;
        std::vector<cv::Mat> r_vecs, t_vecs;
        std::vector<double> std_intrinsics, std_extrinsics, per_view_errors;

        // calibration
        const auto rms = cv::calibrateCamera(
                object_points,
                corners,
                cv::Size(width, height),
                camera_matrix,
                distortion_coefficients,
                r_vecs,
                t_vecs,
                std_intrinsics,
                std_extrinsics,
                per_view_errors
        );

        return {
                .camera_matrix = camera_matrix,
                .distortion_coefficients = distortion_coefficients,
                .rotation_vecs = r_vecs,
                .translation_vecs = t_vecs,
                .std_dev_intrinsics = std_intrinsics,
                .std_dev_extrinsics = std_extrinsics,
                .per_view_errors = per_view_errors,
                .rms = rms
        };
    }

}