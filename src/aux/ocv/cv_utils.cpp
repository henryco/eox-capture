//
// Created by henryco on 12/3/23.
//

#include "cv_utils.h"

#include <cmath>

#include <opencv2/imgproc.hpp>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
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
                          | cv::CALIB_CB_NORMALIZE_IMAGE
                          | cv::CALIB_CB_FILTER_QUADS
                          | cv::CALIB_CB_ADAPTIVE_THRESH
                          | cv::CALIB_CB_ACCURACY
                          | cv::CALIB_CB_EXHAUSTIVE
                          ;

        std::vector<cv::Point2f> corners;
        const bool found = cv::findChessboardCorners(gray, size, corners, flags);

        if (found) {
            const auto term = cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.001);
            const auto window = cv::Size(11, 11);
            const auto zone = cv::Size(-1, -1);
            cv::cornerSubPix(gray, corners, window, zone, term);
        }

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


        // calculating mean re-projection error
        double totalError = 0;
        size_t totalPoints = 0;
        std::vector<cv::Point2f> reprojectedPoints;
        for (size_t i = 0; i < object_points.size(); ++i) {
            projectPoints(
                    object_points[i],
                    r_vecs[i],
                    t_vecs[i],
                    camera_matrix,
                    distortion_coefficients,
                    reprojectedPoints
            );
            size_t n = object_points[i].size();
            for (size_t j = 0; j < n; ++j) {
                double err = norm(corners[i][j] - reprojectedPoints[j]);
                totalError += err * err;
            }
            totalPoints += n;
        }
        double mre = sqrt(totalError / (double) totalPoints);


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
                .mre = mre,
                .width = width,
                .height = height,
                .uid = id
        };
    }

    CalibrationStereo
    calibrate_stereo_pair(std::vector<std::vector<cv::Point2f>> &corners_l, CalibrationSolo &calibration_l,
                          std::vector<std::vector<cv::Point2f>> &corners_r, CalibrationSolo &calibration_r,
                          uint width, uint height, uint rows, uint cols, bool correction) {
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
        cv::Mat per_view_errors;

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
                per_view_errors,
                correction ? cv::CALIB_USE_INTRINSIC_GUESS : cv::CALIB_FIX_INTRINSIC
        );

        // result
        return {
                .R = R,
                .T = T,
                .E = E,
                .F = F,
                .per_view_errors = per_view_errors,
                .rms = rms,
                .width = width,
                .height = height
        };
    }

    CalibrationStereo
    calibrate_stereo_pair(std::vector<std::vector<cv::Point2f>> &corners_l, CalibrationSolo &calibration_l,
                          std::vector<std::vector<cv::Point2f>> &corners_r, CalibrationSolo &calibration_r,
                          uint rows, uint cols, bool correction) {
        return calibrate_stereo_pair(
                corners_l,
                calibration_l,
                corners_r,
                calibration_r,
                calibration_l.width,
                calibration_l.height,
                rows,
                cols,
                correction);
    }

    StereoRectification rectify_stereo(
            CalibrationSolo &calibration_l,
            CalibrationSolo &calibration_r,
            CalibrationStereo &stereo,
            double alpha
    ) {
        const auto img_size = cv::Size((int) stereo.width, (int) stereo.height);

        // output parameters
        cv::Mat R1, R2, P1, P2, Q, L_MAP1, L_MAP2, R_MAP1, R_MAP2;
        auto roi_l = std::make_unique<cv::Rect2i>();
        auto roi_r = std::make_unique<cv::Rect2i>();

        // rectification
        cv::stereoRectify(
                calibration_l.camera_matrix,
                calibration_l.distortion_coefficients,
                calibration_r.camera_matrix,
                calibration_r.distortion_coefficients,
                img_size,
                stereo.R,
                stereo.T,
                R1, R2, P1, P2, Q,
                cv::CALIB_ZERO_DISPARITY,
                alpha,
                cv::Size(),
                roi_l.get(),
                roi_r.get()
        );

        cv::initUndistortRectifyMap(
                calibration_l.camera_matrix,
                calibration_l.distortion_coefficients,
                R1,
                P1,
                img_size,
                CV_16SC2, // CV_32FC2 for high accuracy
                L_MAP1,
                L_MAP2
        );

        cv::initUndistortRectifyMap(
                calibration_r.camera_matrix,
                calibration_r.distortion_coefficients,
                R2,
                P2,
                img_size,
                CV_16SC2, // CV_32FC2 for high accuracy
                R_MAP1,
                R_MAP2
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
                .L_MAP1 = L_MAP1,
                .L_MAP2 = L_MAP2,
                .R_MAP1 = R_MAP1,
                .R_MAP2 = R_MAP2
        };
    }

    void write_stereo_package(const StereoPackage &package, const std::string &file_name, bool b64) {
        const auto flags = cv::FileStorage::WRITE | (b64 ? cv::FileStorage::BASE64 : 0);
        cv::FileStorage fs(file_name, flags);

        // SOLO
        {
            fs << "type" << "eox::calibration";
            fs << "devices" << ((int) package.solo.size());
            int i = 0;
            for (const auto &[k, solo]: package.solo) {
                const std::string index = "s_" + std::to_string(i);
                fs << index + "_cm" << solo.camera_matrix;
                fs << index + "_dc" << solo.distortion_coefficients;
                fs << index + "_rv" << solo.rotation_vecs;
                fs << index + "_tv" << solo.translation_vecs;
                fs << index + "_di" << solo.std_dev_intrinsics;
                fs << index + "_de" << solo.std_dev_extrinsics;
                fs << index + "_pe" << solo.per_view_errors;
                fs << index + "_er" << solo.rms;
                fs << index + "_me" << solo.mre;
                fs << index + "_wh" << (int) solo.width;
                fs << index + "_hg" << (int) solo.height;
                fs << index + "_id" << (int) solo.uid;
                i++;
            }
        }

        // STEREO
        {
            const std::string index = "x";
            fs << index + "_r" << package.stereo.R;
            fs << index + "_t" << package.stereo.T;
            fs << index + "_e" << package.stereo.E;
            fs << index + "_f" << package.stereo.F;
            fs << index + "_p" << package.stereo.per_view_errors;
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
            fs << index + "_1l" << package.rectification.L_MAP1;
            fs << index + "_2l" << package.rectification.L_MAP2;
            fs << index + "_1r" << package.rectification.R_MAP1;
            fs << index + "_2r" << package.rectification.R_MAP2;
        }

        fs.release();
    }

    StereoPackage read_stereo_package(const std::string &file_name) {
        StereoPackage package;
        cv::FileStorage fs(file_name, cv::FileStorage::READ);

        std::string type;
        fs["type"] >> type;

        if ("eox::calibration" != type) {
            fs.release();
            return {
                    .ok = false
            };
        }

        // SOLO
        {
            std::map<uint, CalibrationSolo> solo;

            int devices = 0;
            fs["devices"] >> devices;

            for (int i = 0; i < devices; i++) {
                const std::string index = "s_" + std::to_string(i);

                cv::Mat camera_matrix;
                cv::Mat distortion_coefficients;
                std::vector<cv::Mat> rotation_vecs;
                std::vector<cv::Mat> translation_vecs;
                std::vector<double> std_dev_intrinsics;
                std::vector<double> std_dev_extrinsics;
                std::vector<double> per_view_errors;
                double rms = NAN;
                double mre = NAN;
                int width = 0;
                int height = 0;
                int uid = 0;

                fs[index + "_cm"] >> camera_matrix;
                fs[index + "_dc"] >> distortion_coefficients;
                fs[index + "_rv"] >> rotation_vecs;
                fs[index + "_tv"] >> translation_vecs;
                fs[index + "_di"] >> std_dev_intrinsics;
                fs[index + "_de"] >> std_dev_extrinsics;
                fs[index + "_pe"] >> per_view_errors;
                fs[index + "_er"] >> rms;
                fs[index + "_me"] >> mre;
                fs[index + "_wh"] >> width;
                fs[index + "_hg"] >> height;
                fs[index + "_id"] >> uid;

                const CalibrationSolo instance = {
                        .camera_matrix = camera_matrix,
                        .distortion_coefficients = distortion_coefficients,
                        .rotation_vecs = rotation_vecs,
                        .translation_vecs = translation_vecs,
                        .std_dev_intrinsics = std_dev_intrinsics,
                        .std_dev_extrinsics = std_dev_extrinsics,
                        .per_view_errors = per_view_errors,
                        .rms = rms,
                        .mre = mre,
                        .width = (uint) width,
                        .height = (uint) height,
                        .uid = (uint) uid
                };

                solo.emplace((uint) uid, instance);
            }

            package.solo = solo;
        }

        // STEREO
        {
            cv::Mat R;
            cv::Mat T;
            cv::Mat E;
            cv::Mat F;
            cv::Mat per_view_errors;
            double rms = NAN;
            int width = 0;
            int height = 0;

            fs["x_r"] >> R;
            fs["x_t"] >> T;
            fs["x_e"] >> E;
            fs["x_f"] >> F;
            fs["x_p"] >> per_view_errors;
            fs["x_s"] >> rms;
            fs["x_w"] >> width;
            fs["x_h"] >> height;

            package.stereo = {
                    .R = R,
                    .T = T,
                    .E = E,
                    .F = F,
                    .per_view_errors = per_view_errors,
                    .rms = rms,
                    .width = (uint) width,
                    .height = (uint) height
            };
        }

        // RECTIFICATION
        {
            cv::Mat R1;
            cv::Mat R2;
            cv::Mat P1;
            cv::Mat P2;
            cv::Mat Q;
            cv::Rect2i ROI_L;
            cv::Rect2i ROI_R;
            cv::Mat L_MAP1;
            cv::Mat L_MAP2;
            cv::Mat R_MAP1;
            cv::Mat R_MAP2;
            cv::Mat NL;
            cv::Mat NR;

            fs["r_r1"] >> R1;
            fs["r_r2"] >> R2;
            fs["r_p1"] >> P1;
            fs["r_p2"] >> P2;
            fs["r_qq"] >> Q;
            fs["r_i1"] >> ROI_L;
            fs["r_i2"] >> ROI_R;
            fs["r_1l"] >> L_MAP1;
            fs["r_2l"] >> L_MAP2;
            fs["r_1r"] >> R_MAP1;
            fs["r_2r"] >> R_MAP2;

            package.rectification = {
                    .R1 = R1,
                    .R2 = R2,
                    .P1 = P1,
                    .P2 = P2,
                    .Q = Q,
                    .ROI_L = ROI_L,
                    .ROI_R = ROI_R,
                    .L_MAP1 = L_MAP1,
                    .L_MAP2 = L_MAP2,
                    .R_MAP1 = R_MAP1,
                    .R_MAP2 = R_MAP2
            };
        }

        fs.release();

        package.ok = true;
        return package;
    }

    void write_stereo_matcher(const cv::StereoMatcher *const matcher, const std::string &file_name, bool b64) {
        const auto flags = cv::FileStorage::WRITE | (b64 ? cv::FileStorage::BASE64 : 0);
        cv::FileStorage fs(file_name, flags);
        {
            if (dynamic_cast<const cv::StereoBM *>(matcher)) {
                fs << "eox::type" << "BM";
            }
            else if (dynamic_cast<const cv::StereoSGBM *>(matcher)) {
                fs << "eox::type" << "SGBM";
            }
            else {
                fs.release();
                throw std::runtime_error("unknown stereo matcher type");
            }
            matcher->write(fs);
        }
        fs.release();
    }

    bool read_stereo_matcher(cv::StereoMatcher *matcher, const std::string &file_name) {
        cv::FileStorage fs(file_name, cv::FileStorage::READ);

        std::string type;
        fs["eox::type"] >> type;

        if ("BM" == type && dynamic_cast<cv::StereoBM *>(matcher)) {
            matcher->read(fs.root());
            fs.release();
            return true;
        }

        else if ("SGBM" == type && dynamic_cast<cv::StereoSGBM *>(matcher)) {
            matcher->read(fs.root());
            fs.release();
            return true;
        }

        fs.release();
        return false;
    }
}
#pragma clang diagnostic pop