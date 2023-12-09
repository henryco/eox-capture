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

            // unpacking left and right frame
            auto &frame_l = frames_pair[0];
            auto &frame_r = frames_pair[1];

            // remapping frames according to stereo rectification
            const auto &rect = packages.at(g_id).rectification;
            cv::Mat rect_l, rect_r;
            cv::remap(frame_l, rect_l, rect.L_MAP1, rect.L_MAP2, cv::INTER_CUBIC);
            cv::remap(frame_r, rect_r, rect.R_MAP1, rect.R_MAP2, cv::INTER_CUBIC);

            // TODO MAGIC

            // saving results to global vector of frames (goes to render output)
            _frames.push_back(frame_l);
            _frames.push_back(frame_r);
        }

        glImage.setFrames(_frames);
        refresh();
    }

#pragma clang diagnostic pop

}