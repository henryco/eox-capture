//
// Created by henryco on 12/3/23.
//

#ifndef STEREOX_CV_UTILS_H
#define STEREOX_CV_UTILS_H

#include <opencv2/core/mat.hpp>
#include <opencv4/opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

namespace eox::ocv {

    typedef struct {
        std::vector<cv::Point2f> corners;
        cv::Mat original;
        cv::Mat result;
        bool found;
    } Squares;

    typedef struct {
        cv::Mat camera_matrix;
        cv::Mat distortion_coefficients;
        std::vector<cv::Mat> rotation_vecs;
        std::vector<cv::Mat> translation_vecs;
        std::vector<double> std_dev_intrinsics;
        std::vector<double> std_dev_extrinsics;
        std::vector<double> per_view_errors;

        double rms;
        int width;
        int height;
    } CalibrationSolo;

    typedef struct {
        cv::Mat R;
        cv::Mat T;
        cv::Mat E;
        cv::Mat F;

        double rms;
        int width;
        int height;
    } CalibrationStereo;

    typedef struct {
        cv::Mat R1;
        cv::Mat R2;
        cv::Mat P1;
        cv::Mat P2;
        cv::Mat Q;

        cv::Rect2i ROI_L;
        cv::Rect2i ROI_R;
    } StereoRectification;


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
            int columns,
            int rows,
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
            int width,
            int height,
            int rows,
            int columns);


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
            int rows,
            int columns
    );


    /**
     * @note Overloaded
     */
    CalibrationStereo calibrate_stereo_pair(
            std::vector<std::vector<cv::Point2f>> &corners_left,
            CalibrationSolo &calibration_left,
            std::vector<std::vector<cv::Point2f>> &corners_right,
            CalibrationSolo &calibration_right,
            int width,
            int height,
            int rows,
            int columns
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
}


#endif //STEREOX_CV_UTILS_H
