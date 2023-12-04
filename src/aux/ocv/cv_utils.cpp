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

    Squares find_squares(const cv::Mat &image, uint columns, uint rows, int flag) {
        cv::Mat gray = img_copy(image, cv::COLOR_BGR2GRAY);
        cv::Mat copy = img_copy(image);

        const auto size = cv::Size((int) columns - 1, (int) rows - 1);
        const int flags = flag
                          | cv::CALIB_CB_FAST_CHECK
                          | cv::CALIB_CB_NORMALIZE_IMAGE
                          | cv::CALIB_CB_FILTER_QUADS
                          | cv::CALIB_CB_ADAPTIVE_THRESH;

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
    calibrate_solo(std::vector<std::vector<cv::Point2f>> &corners, uint id, uint width, uint height, uint rows,
                   uint cols) {
        // Prepare object points (0,0,0), (1,0,0), (2,0,0) ... (8,5,0)
        std::vector<cv::Point3f> obj_p;
        for (int i = 0; i < rows - 1; ++i) {
            for (int j = 0; j < cols - 1; ++j) {
                obj_p.emplace_back((float) j, (float) i, 0.0f);
            }
        }

        // Replicate obj_p for each image
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
                cv::Size((int) width, (int) height),
                camera_matrix,
                distortion_coefficients,
                r_vecs,
                t_vecs,
                std_intrinsics,
                std_extrinsics,
                per_view_errors
        );

        // result
        return {
                .camera_matrix = camera_matrix,
                .distortion_coefficients = distortion_coefficients,
                .rotation_vecs = r_vecs,
                .translation_vecs = t_vecs,
                .std_dev_intrinsics = std_intrinsics,
                .std_dev_extrinsics = std_extrinsics,
                .per_view_errors = per_view_errors,
                .rms = rms,
                .width = width,
                .height = height,
                .uid = id
        };
    }

    CalibrationStereo
    calibrate_stereo_pair(std::vector<std::vector<cv::Point2f>> &corners_l, CalibrationSolo &calibration_l,
                          std::vector<std::vector<cv::Point2f>> &corners_r, CalibrationSolo &calibration_r,
                          uint width, uint height, uint rows, uint cols) {
        // Prepare object points (0,0,0), (1,0,0), (2,0,0) ... (8,5,0)
        std::vector<cv::Point3f> obj_p;
        for (int i = 0; i < rows - 1; ++i) {
            for (int j = 0; j < cols - 1; ++j) {
                obj_p.emplace_back((float) j, (float) i, 0.0f);
            }
        }

        // Replicate obj_p for each image
        std::vector<std::vector<cv::Point3f>> object_points;
        object_points.reserve(corners_l.size());
        // L and R should be the same size anyway
        for (int i = 0; i < corners_l.size(); ++i) {
            object_points.push_back(obj_p);
        }

        // output parameters
        cv::Mat R, T, E, F;

        // calibration
        auto const rms = cv::stereoCalibrate(
                object_points,
                corners_l,
                corners_r,
                calibration_l.camera_matrix,
                calibration_l.distortion_coefficients,
                calibration_r.camera_matrix,
                calibration_r.distortion_coefficients,
                cv::Size((int) width, (int) height),
                R, T, E, F,
                cv::CALIB_FIX_INTRINSIC
        );

        // result
        return {
                .R = R,
                .T = T,
                .E = E,
                .F = F,
                .rms = rms,
                .width = width,
                .height = height
        };
    }

    CalibrationStereo
    calibrate_stereo_pair(std::vector<std::vector<cv::Point2f>> &corners_l, CalibrationSolo &calibration_l,
                          std::vector<std::vector<cv::Point2f>> &corners_r, CalibrationSolo &calibration_r,
                          uint rows, uint cols) {
        return calibrate_stereo_pair(
                corners_l,
                calibration_l,
                corners_r,
                calibration_r,
                calibration_l.width,
                calibration_l.height,
                rows,
                cols);
    }

    StereoRectification rectify_stereo(
            CalibrationSolo &calibration_l,
            CalibrationSolo &calibration_r,
            CalibrationStereo &stereo,
            double alpha
    ) {

        // output parameters
        cv::Mat R1, R2, P1, P2, Q;
        auto roi_l = std::make_unique<cv::Rect2i>();
        auto roi_r = std::make_unique<cv::Rect2i>();

        // rectification
        cv::stereoRectify(
                calibration_l.camera_matrix,
                calibration_l.distortion_coefficients,
                calibration_r.camera_matrix,
                calibration_r.distortion_coefficients,
                cv::Size((int) stereo.width, (int) stereo.height),
                stereo.R,
                stereo.T,
                R1, R2, P1, P2, Q,
                cv::CALIB_ZERO_DISPARITY,
                alpha,
                cv::Size(),
                roi_l.get(),
                roi_r.get()
        );

        // result
        return {
                .R1 = R1,
                .R2 = R2,
                .P1 = P1,
                .P2 = P2,
                .Q = Q,
                .ROI_L = cv::Rect2i(*roi_l),
                .ROI_R = cv::Rect2i(*roi_r),
        };
    }

    void write_stereo_package(const StereoPackage &package, const std::string &file_name, bool b64) {
        const auto flags = cv::FileStorage::WRITE | (b64 ? cv::FileStorage::BASE64 : 0);
        cv::FileStorage fs(file_name, flags);

        // SOLO
        {
            fs << "devices" << ((int) package.solo.size());
            for (const auto &[k, solo]: package.solo) {
                const std::string index = "s_" + std::to_string(k);
                fs << index + "_cm" << solo.camera_matrix;
                fs << index + "_dc" << solo.distortion_coefficients;
                fs << index + "_rv" << solo.rotation_vecs;
                fs << index + "_tv" << solo.translation_vecs;
                fs << index + "_di" << solo.std_dev_intrinsics;
                fs << index + "_de" << solo.std_dev_extrinsics;
                fs << index + "_pe" << solo.per_view_errors;
                fs << index + "_er" << solo.rms;
                fs << index + "_wh" << (int) solo.width;
                fs << index + "_hg" << (int) solo.height;
                fs << index + "_id" << (int) solo.uid;
            }
        }

        // STEREO
        {
            const std::string index = "x";
            fs << index + "_r" << package.stereo.R;
            fs << index + "_t" << package.stereo.T;
            fs << index + "_e" << package.stereo.E;
            fs << index + "_f" << package.stereo.F;
            fs << index + "_s" << package.stereo.rms;
            fs << index + "_w" << (int) package.stereo.width;
            fs << index + "_h" << (int) package.stereo.height;
        }

        // RECTIFICATION
        {
            const std::string index = "r";
            fs << index + "_r1" << package.rectification.R1;
            fs << index + "_r2" << package.rectification.R2;
            fs << index + "_p1" << package.rectification.P1;
            fs << index + "_p2" << package.rectification.P2;
            fs << index + "_qq" << package.rectification.Q;
            fs << index + "_i1" << package.rectification.ROI_L;
            fs << index + "_i2" << package.rectification.ROI_R;
        }

        fs.release();
    }
}