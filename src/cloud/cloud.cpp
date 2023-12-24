//
// Created by henryco on 12/8/23.
//

#include "ui_points_cloud.h"

#include <opencv2/photo.hpp>

namespace eox {

    using frame_container = struct {
        std::map<ts::device_id, cv::Mat> frames;
    };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"

    void UiPointsCloud::update(float delta, float late, float fps) {
        FPS = fps;

        auto captured = camera.captureWithId();
        if (captured.empty()) {
            log->debug("nothing captured, skip");
            return;
        }

        std::map<ts::group_id, frame_container> frames;
        for (const auto &[d_id, frame]: captured) {
            auto group_id = deviceGroupMap.at(d_id);
            auto &container = frames[group_id];
            container.frames.emplace(d_id, frame);
        }

        // vector with output frames (goes to render)
        std::vector<cv::Mat> _frames;

        // iterating over each device group (main magic here)
        for (const auto &[g_id, container]: frames) {

            // unpacking frames container
            std::vector<cv::UMat> frames_pair;
            frames_pair.reserve(container.frames.size());
            for (const auto &[d_id, frame]: container.frames) {
                cv::UMat dst;
                frame.copyTo(dst);
                frames_pair.push_back(dst);
            }

            // up/downscale
            const auto &c_conf = config.camera[0];
            if (c_conf.output_width != c_conf.width
                && c_conf.output_height != c_conf.height) {
                const auto new_size = cv::Size(c_conf.output_width, c_conf.output_height);
                const auto type = c_conf.output_width < c_conf.width ? cv::INTER_AREA : cv::INTER_CUBIC;
                for (auto &frame: frames_pair) {
                    cv::UMat u_dst;
                    cv::resize(frame, u_dst, new_size, 0, 0, type);
                    frame = u_dst;
                }
            }

            // unpacking rectification maps to GPU matrices
            const auto &rect = packages.at(g_id).rectification;
            cv::UMat L_MAP1, L_MAP2, R_MAP1, R_MAP2;
            rect.L_MAP1.copyTo(L_MAP1);
            rect.L_MAP2.copyTo(L_MAP2);
            rect.R_MAP1.copyTo(R_MAP1);
            rect.R_MAP2.copyTo(R_MAP2);

            // unpacking left and right frames to GPU matrices
            cv::UMat &frame_l = frames_pair[0];
            cv::UMat &frame_r = frames_pair[1];

            // remapping frames according to stereo rectification
            cv::UMat source_l, source_r;
            cv::remap(frame_l, source_l, L_MAP1, L_MAP2, cv::INTER_LINEAR);
            cv::remap(frame_r, source_r, R_MAP1, R_MAP2, cv::INTER_LINEAR);

            // convert from BGR to Grayscale
            cv::UMat gray_l, gray_r;
            cv::cvtColor(source_l, gray_l, cv::COLOR_BGR2GRAY);
            cv::cvtColor(source_r, gray_r, cv::COLOR_BGR2GRAY);

            // denoising, might be very resource intensive
            if (config.denoise) {
                cv::UMat filtered_l, filtered_r;
                cv::fastNlMeansDenoising(gray_l, filtered_l);
                cv::fastNlMeansDenoising(source_r, filtered_r);
                gray_l = filtered_l;
                gray_r = filtered_r;
            }

            // computing disparity map
            cv::UMat disparity, disparity_raw;
            if (config.stereo.confidence) {
                cv::UMat disparity_l;
                cv::UMat disparity_r;

                matchers.at(g_id).first->compute(gray_l, gray_r, disparity_l);
                matchers.at(g_id).second->compute(gray_r, gray_l, disparity_r);

                // Filter Speckles
                //cv::filterSpeckles(disparity_l, 0, 32, 25);
                //cv::filterSpeckles(disparity_r, 0, 32, 25);

                disparity_raw = disparity_l;

                if (wlsFilters.at(g_id)->getLambda() != 0) {
                    wlsFilters.at(g_id)->filter(
                            disparity_l,
                            gray_l,
                            disparity,
                            disparity_r,
                            cv::Rect(),
                            gray_r
                    );
                } else {
                    disparity = disparity_raw;
                }
            } else {

                matchers.at(g_id).first->compute(gray_l, gray_r, disparity_raw);

                // Filter Speckles
                //cv::filterSpeckles(disparity_raw, 0, 32, 25);

                if (wlsFilters.at(g_id)->getLambda() != 0) {
                    wlsFilters.at(g_id)->filter(
                            disparity_raw,
                            gray_l,
                            disparity
                    );
                } else {
                    disparity = disparity_raw;
                }
            }

            // converting to CV_16F
            if ((disparity.depth() & CV_MAT_DEPTH_MASK) == CV_16S) {
                cv::UMat temp;
                disparity.convertTo(temp, CV_32F, 1. / 16.);
                disparity = temp;
            }


            // ! ! !
            // NOTE, SHOULD USE [ disparity ] matrix for further computation
            // ! ! !

            cv::UMat points_cloud;
            cv::reprojectImageTo3D(disparity, points_cloud, rect.Q, true);

            // ! ! !
            // NOTE, SHOULD USE [ points_cloud ] matrix for further computation
            // ! ! !


            // Convert the disparity values to a range that can be represented in 8-bit format
            cv::UMat normalized_disp, normalized_raw, normalized_point;
            cv::normalize(disparity, normalized_disp, 0, 255, cv::NORM_MINMAX, CV_8U);
            cv::normalize(disparity_raw, normalized_raw, 0, 255, cv::NORM_MINMAX, CV_8U);

            {
                cv::UMat depth, temp;
                cv::extractChannel(points_cloud, depth, 2);
                eox::ocv::clamp(depth, 0, 255);
                depth.convertTo(temp, CV_8U);
                cv::applyColorMap(temp, normalized_point, cv::COLORMAP_JET);
            }

            // converting back to BGR
            cv::UMat bgr_l, bgr_disparity, bgr_raw;
            cv::cvtColor(normalized_disp, bgr_disparity, cv::COLOR_GRAY2BGR);
            cv::cvtColor(normalized_raw, bgr_raw, cv::COLOR_GRAY2BGR);

            // converting back to regular cv::Mat
            cv::Mat left, raw, disp, point;
            source_l.copyTo(left);
            bgr_raw.copyTo(raw);
            bgr_disparity.copyTo(disp);
            normalized_point.copyTo(point);

            // saving results to global vector of frames (goes to render output)
            _frames.push_back(left);
            _frames.push_back(raw);
            _frames.push_back(disp);
            _frames.push_back(point);

            // assign to member properties
            points[g_id] = ocv::PointCloud(disparity, points_cloud, source_l);
        }

        if (aux) {
            glImage.setFrames(_frames);
        } else {
            for (const auto &[_, p]: points) {
                // TODO FIXME, works only for one group
                cv::Mat pos, col;
                p.points.copyTo(pos);
                p.colors.copyTo(col);
                voxelArea.setPoints(pos, col);
            }
        }

        refresh();
    }

#pragma clang diagnostic pop

}