//
// Created by henryco on 12/3/23.
//

#ifndef STEREOX_CV_UTILS_H
#define STEREOX_CV_UTILS_H

#include <opencv2/core/mat.hpp>
#include <opencv4/opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <map>
#include <opencv2/ximgproc/disparity_filter.hpp>

namespace eox::ocv {

    using Squares = struct {
        std::vector<cv::Point2f> corners;
        cv::Mat original;
        cv::Mat result;
        bool found;
    };

    using CalibrationSolo = struct {
        cv::Mat camera_matrix;
        cv::Mat distortion_coefficients;
        std::vector<cv::Mat> rotation_vecs;
        std::vector<cv::Mat> translation_vecs;
        std::vector<double> std_dev_intrinsics;
        std::vector<double> std_dev_extrinsics;
        std::vector<double> per_view_errors;

        double rms;
        double mre;
        uint width;
        uint height;
        uint uid;
    };

    using CalibrationStereo = struct {
        cv::Mat R;
        cv::Mat T;
        cv::Mat E;
        cv::Mat F;

        cv::Mat per_view_errors;

        double rms;
        uint width;
        uint height;
    };

    using StereoRectification = struct {
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
    };

    using StereoPackage = struct {
        std::map<uint, CalibrationSolo> solo;
        CalibrationStereo stereo;
        StereoRectification rectification;
        bool ok;
    };


    /**
     * \brief Create a copy of the given image.
     * \param[in] image The image to be copied.
     * \return A copy of the given image.
     *
     * This function creates a new instance of `cv::Mat` that is a copy of the given `image`.
     * The copy is created by allocating new memory and copying the pixel data from the original image.
     * The resulting copy can be used independently and modifications to the copy will not affect the original image.
     *
     * Example usage:
     * \code
     * cv::Mat originalImage = cv::imread("input.jpg");
     * cv::Mat imageCopy = img_copy(originalImage);
     * cv::imshow("Original Image", originalImage);
     * cv::imshow("Copied Image", imageCopy);
     * \endcode
     *
     * \note This function requires the OpenCV library to be installed and properly linked.
     * \note The copy operation can consume a significant amount of memory if working with large images.
     */
    cv::Mat img_copy(const cv::Mat &image);


    /**
     * @brief Copies an image with an optional color space conversion.
     *
     * This function takes an input image and creates a copy of it, with an optional
     * color space conversion. The original input image is not modified.
     *
     * @param image The input image to be copied.
     * @param color_space_conv_type The color space conversion type. Use the appropriate
     *     constant from the cv::ColorConversionCodes enumeration defined in OpenCV.
     *     If no conversion is required, use cv::COLOR_NONE.
     *
     * @return The copied image with color space conversion.
     */
    cv::Mat img_copy(const cv::Mat &image, int color_space_conv_type);


    /**
     * @brief img_copy - Copies an image and converts the color space and matrix data type.
     *
     * This function takes an image as input and creates a copy of it. It also allows for converting
     * the color space and matrix data type of the copied image based on the provided parameters.
     *
     * @param image The input image to be copied.
     * @param color_space_conv_type The desired color space conversion type.
     *        Use predefined values from OpenCV cvtColor() function, e.g. CV_BGR2GRAY, CV_BGR2HSV, etc.
     * @param matrix_data_type The desired matrix data type for the copied image.
     *        Use predefined values from OpenCV CV_8U, CV_16U, CV_32F, etc.
     *
     * @return The copied image with the specified color space and matrix data type.
     */
    cv::Mat img_copy(
            const cv::Mat &image,
            int color_space_conv_type,
            int matrix_data_type);


    /**
     * @brief Finds and marks squares in a given image, typically used for chessboard corner detection.
     *
     * This function applies various image processing techniques to detect square patterns within an image,
     * such as those found on a chessboard. It converts the image to grayscale, applies adaptive thresholding,
     * and other filters to facilitate corner detection. Chessboard corners are then identified, and the squares are
     * marked on a copy of the original image.
     *
     * @param image The source image as a cv::Mat object, in which squares are to be detected.
     * @param columns The number of inner corners per a chessboard row.
     * @param rows The number of inner corners per a chessboard column.
     * @param flag Additional flags to control the corner detection algorithm. These flags are combined with
     *        predefined flags such as cv::CALIB_CB_FAST_CHECK, cv::CALIB_CB_NORMALIZE_IMAGE,
     *        cv::CALIB_CB_FILTER_QUADS, and cv::CALIB_CB_ADAPTIVE_THRESH.
     *
     * @return Squares A structure containing the detected corners as a vector of cv::Point2f,
     *         a copy of the original image with drawn corners, and a boolean indicating
     *         whether the corners were found successfully.
     */
    Squares find_squares(
            const cv::Mat &image,
            uint columns,
            uint rows,
            int flag);


    /**
     * @brief Performs camera calibration using a set of 2D-3D point correspondences.
     *
     * This function calibrates a camera using the provided image corners. It prepares object points
     * in a grid pattern and replicates them for each image in the corners vector. Then, it uses
     * OpenCV's calibrateCamera function to compute the camera matrix, distortion coefficients,
     * rotation vectors, translation vectors, standard deviations for intrinsics, extrinsics,
     * per view errors, and the root mean square (RMS) error of re-projection.
     *
     * @param corners A reference to a vector of vectors containing image corner points (cv::Point2f).
     *                These are typically obtained from calibration patterns like checkerboards.
     * @param uid Unique camera id
     * @param width The width of the calibration image in pixels.
     * @param height The height of the calibration image in pixels.
     * @param rows The number of rows in the calibration pattern.
     * @param columns The number of columns in the calibration pattern.
     *
     * @return A structure containing the calibration results:
     *         - camera_matrix: The camera matrix.
     *         - distortion_coefficients: Lens distortion coefficients.
     *         - rotation_vecs: Rotation vectors for each calibration image.
     *         - translation_vecs: Translation vectors for each calibration image.
     *         - std_dev_intrinsics: Standard deviations of intrinsic parameters.
     *         - std_dev_extrinsics: Standard deviations of extrinsic parameters.
     *         - per_view_errors: Per-view re-projection errors.
     *         - rms: The overall root mean square (RMS) re-projection error.
     */
    CalibrationSolo calibrate_solo(
            std::vector<std::vector<cv::Point2f>> &corners,
            uint uid,
            uint width,
            uint height,
            uint rows,
            uint columns);


    /**
     * @brief Performs stereo camera calibration using the provided image corner points and individual camera calibrations.
     *
     * This function calibrates a stereo camera setup by using the corner points from a set of stereo images and the
     * calibration data of each individual camera. It prepares object points in a 3D space for a standard chessboard-like pattern,
     * then performs stereo calibration to find the rotation (R), translation (T), essential (E), and fundamental (F) matrices.
     *
     * @param corners_left A reference to a vector of vectors containing the 2D image points from the left camera.
     * @param calibration_left A reference to the CalibrationSolo object for the left camera, containing its calibration data.
     * @param corners_right A reference to a vector of vectors containing the 2D image points from the right camera.
     * @param calibration_right A reference to the CalibrationSolo object for the right camera, containing its calibration data.
     * @param width The width of the image used for calibration.
     * @param height The height of the image used for calibration.
     * @param rows The number of rows in the chessboard pattern used for calibration (actual number of corners is rows - 1).
     * @param columns The number of columns in the chessboard pattern used for calibration (actual number of corners is cols - 1).
     * @param correction Optimize some of all of the camera intrinsic parameters.
     *
     * @return A CalibrationStereo object containing the rotation matrix (R), translation vector (T), essential matrix (E),
     *         and fundamental matrix (F) of the stereo calibration.
     *
     * @note It's assumed that the vector sizes of corners_l and corners_r are the same as they correspond to pairs of images
     *       from the stereo setup. The chessboard pattern should have (rows - 1) x (cols - 1) corners.
     */
    CalibrationStereo calibrate_stereo_pair(
            std::vector<std::vector<cv::Point2f>> &corners_left,
            CalibrationSolo &calibration_left,
            std::vector<std::vector<cv::Point2f>> &corners_right,
            CalibrationSolo &calibration_right,
            uint rows,
            uint columns,
            bool correction = false
    );


    /**
     * @note Overloaded
     */
    CalibrationStereo calibrate_stereo_pair(
            std::vector<std::vector<cv::Point2f>> &corners_left,
            CalibrationSolo &calibration_left,
            std::vector<std::vector<cv::Point2f>> &corners_right,
            CalibrationSolo &calibration_right,
            uint width,
            uint height,
            uint rows,
            uint columns,
            bool correction = false
    );


    /**
     * @brief Performs stereo rectification on a pair of cameras.
     *
     * This function computes the rectification transformations and projection matrices
     * necessary for stereo image alignment. It uses the calibration data of two cameras
     * and their stereo configuration.
     *
     * @param calibration_left Reference to the calibration data of the left camera (CalibrationSolo).
     * @param calibration_right Reference to the calibration data of the right camera (CalibrationSolo).
     * @param stereo Reference to the stereo configuration data (CalibrationStereo).
     * @param alpha Free scaling parameter. If it is -1 or absent, the function performs the default scaling. Otherwise,
     * the parameter should be between 0 and 1. alpha=0 means that the rectified images are zoomed and shifted so that
     * only valid pixels are visible (no black areas after rectification).
     * alpha=1 means that the rectified image is decimated and shifted so that all the pixels from the original images
     * from the cameras are retained in the rectified images (no source image pixels are lost).
     *
     * @return StereoRectification structure containing rectification matrices and projection matrices.
     *
     * The function relies on OpenCV's cv::stereoRectify function to calculate the rectification parameters.
     * Output matrices R1, R2, P1, P2, and Q represent the rectification rotation matrices for each camera,
     * the projection matrices in the new (rectified) coordinate systems for each camera, and the disparity-to-depth
     * mapping matrix respectively.
     */
    StereoRectification rectify_stereo(
            CalibrationSolo &calibration_left,
            CalibrationSolo &calibration_right,
            CalibrationStereo &stereo,
            double alpha = 0);


    /**
     * @brief Writes stereo camera calibration and rectification data to a specified file.
     *
     * This function serializes the data contained in a StereoPackage object into a file. The file format and
     * encoding can be specified. It supports Base64 encoding for the output file. The function writes various
     * calibration parameters for solo (single camera) and stereo setups, as well as rectification parameters.
     *
     * @param package The StereoPackage object containing stereo camera calibration data.
     * The package includes the following data structures:
     *  - Solo: individual camera calibration data (camera matrix, distortion coefficients, rotation vectors,
     *    translation vectors, standard deviation of intrinsics and extrinsics,
     *    per view errors, RMS error, width, height, UID).
     *  - Stereo: stereo camera pair calibration data (rotation matrix, translation vector, essential matrix,
     *  fundamental matrix, RMS error, width, height).
     *  - Rectification: rectification parameters for stereo camera pair (R1, R2, P1, P2, Q matrices,
     *  ROI for left and right cameras).
     *
     * @param file_name The name of the file to which the data will be written.
     * @param b64 A boolean flag indicating whether to encode the file in Base64.
     * If true, the output is Base64 encoded; otherwise, it's plain text.
     *
     * Usage example:
     *    write_stereo_package(package, "calibration_data.yml", false);
     */
    void write_stereo_package(
            const StereoPackage &package,
            const std::string &file_name,
            bool b64 = false
    );


    /**
     * @brief Reads stereo camera calibration data from a file and returns a StereoPackage object.
     *
     * This function reads calibration data for a stereo camera setup from a specified file. The file should
     * contain calibration data for both solo (single camera) and stereo configurations. The function
     * constructs a StereoPackage object, which includes solo calibration for each device, stereo calibration
     * parameters, and rectification data for each camera.
     *
     * The function uses OpenCV's FileStorage class for reading the data. The expected file format should
     * include data for each solo camera (such as camera matrix, distortion coefficients, rotation and
     * translation vectors, standard deviations for intrinsics and extrinsics, per-view errors, RMS error,
     * image width and height, and a unique identifier for each camera). Stereo parameters include
     * rotation matrix (R), translation vector (T), essential matrix (E), fundamental matrix (F), RMS error,
     * and image dimensions. Rectification parameters include rotation matrices (R1, R2), projection matrices
     * (P1, P2), disparity-to-depth mapping matrix (Q), and regions of interest for left and right cameras (ROI_L, ROI_R).
     *
     * @param file_name The name of the file from which to read the calibration data.
     * @return A StereoPackage object containing the read calibration data.
     *
     * Example usage:
     *
     *     StereoPackage package = read_stereo_package("calibration_data.json");
     *
     * Note: This function assumes the file is correctly formatted and exists. It may throw exceptions
     * if the file is not found or the data is improperly formatted.
     */
    StereoPackage read_stereo_package(const std::string &file_name);


    /**
     * @brief Writes a StereoMatcher object to a file.
     *
     * This function serializes the configuration of a cv::StereoMatcher object (either StereoBM or StereoSGBM)
     * and writes it to a specified file.
     * The file format can optionally be base64 encoded.
     *
     * @param matcher A pointer to a constant cv::StereoMatcher object.
     * This object should be either of type cv::StereoBM or cv::StereoSGBM.
     * @param file_name A string representing the name (and path) of the file where the matcher configuration will be saved.
     * @param b64 A boolean flag indicating whether the file should be base64 encoded. If true, the file is encoded
     * in base64, otherwise, it is saved in plain text.
     *
     * @throw std::runtime_error Throws a runtime error if the matcher type is neither cv::StereoBM nor cv::StereoSGBM.
     *
     * The function determines the type of the matcher object using dynamic casting and writes the type along with
     * the matcher's configuration to the file.
     * If the matcher type is unknown, it releases the file and throws an exception.
     */
    void write_stereo_matcher(
            const cv::StereoMatcher *const matcher,
            const std::string &file_name,
            uint group_id,
            bool b64 = false
    );


    /**
     * @brief Reads and configures a StereoMatcher object from a file.
     *
     * This function deserializes a configuration for a cv::StereoMatcher object (either StereoBM or StereoSGBM)
     * from a specified file and applies this configuration to the provided matcher object.
     *
     * @param matcher A pointer to a non-constant cv::StereoMatcher object. This object should be
     * of type cv::StereoBM or cv::StereoSGBM, matching the type specified in the file.
     * @param file_name A string representing the name (and path) of the file from which the matcher configuration will be read.
     *
     * @return Returns true if the matcher's type matches the type specified in the file and the configuration
     * is successfully applied. Returns false otherwise.
     *
     * The function first reads the type of the matcher from the file and then checks if the provided matcher
     * object is of the same type. If the types match, it applies the configuration from the file to the matcher.
     * If the types do not match, the function returns false without modifying the matcher object.
     */
    bool read_stereo_matcher(
            cv::StereoMatcher *matcher,
            const std::string &file_name,
            uint group_id
    );

    /**
     * @brief Writes a DisparityFilter object to a file.
     *
     * This function serializes the configuration of a cv::ximgproc::DisparityFilter object
     * (specifically DisparityWLSFilter) and writes it to a specified file.
     * The file format can optionally be base64 encoded.
     *
     * @param filter A pointer to a constant cv::ximgproc::DisparityFilter object. This object should be of type
     * cv::ximgproc::DisparityWLSFilter.
     * @param file_name A string representing the name (and path) of the file where the filter configuration will be saved.
     * @param b64 A boolean flag indicating whether the file should be base64 encoded. If true,
     * the file is encoded in base64, otherwise, it is saved in plain text.
     *
     * @throw std::runtime_error Throws a runtime error if the filter type is not cv::ximgproc::DisparityWLSFilter.
     *
     * The function checks the type of the filter object using dynamic casting and writes the type along
     * with the filter's configuration to the file.
     * If the filter type is unknown, it releases the file and throws an exception.
     */
    void write_disparity_filter(
            const cv::ximgproc::DisparityFilter *const filter,
            const std::string &file_name,
            uint group_id,
            bool b64 = false
    );

    /**
     * @brief Reads and configures a DisparityFilter object from a file.
     *
     * This function deserializes a configuration for a cv::ximgproc::DisparityFilter object (specifically DisparityWLSFilter)
     * from a specified file and applies this configuration to the provided filter object.
     *
     * @param filter A pointer to a non-constant cv::ximgproc::DisparityFilter object. This object should be of type
     * cv::ximgproc::DisparityWLSFilter, matching the type specified in the file.
     * @param file_name A string representing the name (and path) of the file from which the filter configuration will be read.
     *
     * @return Returns true if the filter's type matches the type specified in the file and the configuration
     * is successfully applied. Returns false otherwise.
     *
     * The function first reads the type of the filter from the file and then checks if the provided filter object
     * is of the same type. If the types match, it applies the configuration from the file to the filter.
     * If the types do not match, the function returns false without modifying the filter object.
     */
    bool read_disparity_filter(
            cv::ximgproc::DisparityFilter *filter,
            const std::string &file_name,
            uint group_id
    );
}


#endif //STEREOX_CV_UTILS_H
