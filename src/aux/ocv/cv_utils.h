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
        cv::Mat result;
        bool found;
    } Squares;


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

    cv::Mat img_copy(const cv::Mat& image);


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

    cv::Mat img_copy(const cv::Mat& image, int color_space_conv_type);

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
            const cv::Mat& image,
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
            const cv::Mat& image,
            int columns,
            int rows,
            int flag);

}


#endif //STEREOX_CV_UTILS_H
