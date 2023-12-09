//
// Created by henryco on 12/8/23.
//

#include "ui_points_cloud.h"

namespace eox {

    using frame_container = struct {
        // key: device_id | value: Frame
        std::map<uint, cv::Mat> frames;
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

        // vector with output frames (goes to render)
        std::vector<cv::Mat> _frames;

        // group_id | frames
        std::map<uint, frame_container> frames;
        for (const auto &[d_id, frame]: captured) {
            auto group_id = deviceGroupMap.at(d_id);
            auto &container = frames[group_id];
            container.frames.emplace(d_id, frame);
        }

        // iterating over each device group (main magic here)
        for (const auto &[g_id, container]: frames) {

            // unpacking frames container
            std::vector<cv::Mat> frames_pair;
            frames_pair.reserve(container.frames.size());
            for (const auto &[d_id, frame]: container.frames) {
                frames_pair.push_back(frame);
            }

            // unpacking rectification maps to GPU matrices
            const auto &rect = packages.at(g_id).rectification;
            cv::UMat L_MAP1, L_MAP2, R_MAP1, R_MAP2;
            rect.L_MAP1.copyTo(L_MAP1);
            rect.L_MAP2.copyTo(L_MAP2);
            rect.R_MAP1.copyTo(R_MAP1);
            rect.R_MAP2.copyTo(R_MAP2);

            // unpacking left and right frames to GPU matrices
            cv::UMat frame_l, frame_r;
            frames_pair[0].copyTo(frame_l);
            frames_pair[1].copyTo(frame_r);

            // remapping frames according to stereo rectification
            cv::UMat rect_l, rect_r;
            cv::remap(frame_l, rect_l, L_MAP1, L_MAP2, cv::INTER_LINEAR);
            cv::remap(frame_r, rect_r, R_MAP1, R_MAP2, cv::INTER_LINEAR);

            // convert from BGR to Grayscale
            cv::UMat gray_l, gray_r, disparity;
            cv::cvtColor(rect_l, gray_l, cv::COLOR_BGR2GRAY);
            cv::cvtColor(rect_r, gray_r, cv::COLOR_BGR2GRAY);

            // computing disparity map
            blockMatcher->compute(gray_l, gray_r, disparity);

            // Filter Speckles
//            cv::filterSpeckles(disparity, 0, 32, 25);

            cv::UMat filtered_disparity;
            wlsFilter->filter(disparity, gray_l, filtered_disparity);



            // ! ! !
            // NOTE, SHOULD USE [ filtered_disparity ] matrix for further computation
            // ! ! !



            // Convert the disparity values to a range that can be represented in 8-bit format
            cv::UMat normalized_disp;
            cv::normalize(filtered_disparity, normalized_disp, 0, 255, cv::NORM_MINMAX, CV_8U);

            // converting back to BGR
            cv::UMat bgr_l, bgr_r, bgr_disparity;
            cv::cvtColor(gray_l, bgr_l, cv::COLOR_GRAY2BGR);
            cv::cvtColor(gray_r, bgr_r, cv::COLOR_GRAY2BGR);
            cv::cvtColor(normalized_disp, bgr_disparity, cv::COLOR_GRAY2BGR);

            // converting back to regular cv::Mat
            cv::Mat left, right, disp;
            bgr_l.copyTo(left);
            bgr_r.copyTo(right);
            bgr_disparity.copyTo(disp);

            // saving results to global vector of frames (goes to render output)
            _frames.push_back(left);
            _frames.push_back(right);
            _frames.push_back(disp);
        }

        glImage.setFrames(_frames);
        refresh();
    }

#pragma clang diagnostic pop

}