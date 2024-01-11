//
// Created by henryco on 1/10/24.
//

#include "pose_pipeline.h"

namespace eox {

    void PosePipeline::init() {
        filters.clear();
        filters.reserve(117); // 39 * (x,y,z) == 39 * 3 == 117
        for (int i = 0; i < 117; i++) {
            filters.emplace_back(f_win_size, f_v_scale, f_fps);
        }
        initialized = true;
    }

    PosePipelineOutput PosePipeline::pass(const cv::Mat &frame) {
        cv::Mat segmentation;
        return pass(frame, segmentation);
    }

    PosePipelineOutput PosePipeline::pass(const cv::Mat &frame, cv::Mat &segmented) {
        return inference(frame, segmented, nullptr);
    }

    PosePipelineOutput PosePipeline::pass(const cv::Mat &frame, cv::Mat &segmented, cv::Mat &debug) {
        return inference(frame, segmented, &debug);
    }

    PosePipelineOutput PosePipeline::inference(const cv::Mat &frame, cv::Mat &segmented, cv::Mat *debug) {
        if (!initialized) {
            init();
        }

        PosePipelineOutput output;
        cv::Mat source;

        if (prediction) {
            // crop using roi
            source = frame(cv::Rect(roi.x, roi.y, roi.w, roi.h));
        } else {
            // use detector model TODO
            log->debug("using pose detector");
            roi = {.x = 0, .y = 0, .w = frame.cols, .h = frame.rows};
            source = frame;
        }

        auto result = pose.inference(source);
        const auto now = timestamp();

        if (result.presence > threshold) {
            eox::dnn::Landmark landmarks[39];
            for (int i = 0; i < 39; i++) {
                landmarks[i] = {
                        // turning x,y into common (global) coordinates
                        .x = (result.landmarks_norm[i].x * roi.w) + roi.x,
                        .y = (result.landmarks_norm[i].y * roi.h) + roi.y,

                        // z is still normalized (in range of 0 and 1)
                        .z = result.landmarks_norm[i].z,

                        .v = result.landmarks_norm[i].v,
                        .p = result.landmarks_norm[i].p,
                };
            }

            // temporal filtering (low pass based on velocity)
            for (int i = 0; i < 39; i++) {
                const auto idx = i * 3;
                auto fx = filters.at(idx + 0).filter(now, landmarks[i].x);
                auto fy = filters.at(idx + 1).filter(now, landmarks[i].y);
                auto fz = filters.at(idx + 2).filter(now, landmarks[i].z);

                landmarks[i].x = fx;
                landmarks[i].y = fy;
                landmarks[i].z = fz;
            }

            performSegmentation(result.segmentation, frame, segmented);

            if (debug) {
                segmented.copyTo(*debug);
                drawJoints(landmarks, *debug);
                drawLandmarks(landmarks, *debug);
                drawRoi(*debug);
            }

            // predict new roi
            roi = roiPredictor.forward(eox::dnn::roiFromPoseLandmarks39(landmarks));
            prediction = true;

            // output
            {
                for (int i = 0; i < 39; i++)
                    output.landmarks[i] = landmarks[i];
                for (int i = 0; i < 128 * 128; i++)
                    output.segmentation[i] = result.segmentation[i];

                output.score = result.presence;
                output.present = true;
            }

        } else {

            // retry but without prediction
            if (prediction) {
                prediction = false;
                return inference(frame, segmented, debug);
            }

            // still nothing
            if (debug)
                frame.copyTo(*debug);
        }

        return output;
    }

    void PosePipeline::performSegmentation(float *segmentation_array, const cv::Mat &frame, cv::Mat &out) const {
        cv::Mat segmentation(128, 128, CV_32F, segmentation_array);
        cv::Mat segmentation_mask;

        cv::threshold(segmentation, segmentation_mask, 0.5, 1., cv::THRESH_BINARY);
        cv::resize(segmentation_mask, segmentation_mask, cv::Size(roi.w, roi.h));

        segmentation_mask.convertTo(segmentation_mask, CV_32FC1, 255.);
        segmentation_mask.convertTo(segmentation_mask, CV_8UC1);

        cv::Mat segmentation_frame = cv::Mat::zeros(frame.rows, frame.cols, CV_8UC1);
        segmentation_mask.copyTo(segmentation_frame(cv::Rect(roi.x, roi.y, roi.w, roi.h)));

        cv::bitwise_and(frame, frame, out, segmentation_frame);
    }

    void PosePipeline::drawJoints(const eox::dnn::Landmark landmarks[39], cv::Mat &output) const {
        for (auto bone: eox::dnn::body_joints) {
            const auto &start = landmarks[bone[0]];
            const auto &end = landmarks[bone[1]];
            if (eox::dnn::sigmoid(start.p) > threshold && eox::dnn::sigmoid(end.p) > threshold) {
                cv::Point sp(start.x, start.y);
                cv::Point ep(end.x, end.y);
                cv::Scalar color(230, 0, 230);
                cv::line(output, sp, ep, color, 2);
            }
        }
    }

    void PosePipeline::drawLandmarks(const eox::dnn::Landmark landmarks[39], cv::Mat &output) const {
        for (int i = 0; i < 39; i++) {
            const auto point = landmarks[i];
            if (eox::dnn::sigmoid(point.v) > threshold || i > 32) {
                cv::Point circle(point.x, point.y);
                cv::Scalar color(0, 255, 230);
                cv::circle(output, circle, 2, color, 1);
                if (i > 32)
                    cv::putText(output, std::to_string(i), cv::Point(circle.x - 10, circle.y - 10),
                                cv::FONT_HERSHEY_PLAIN, 1,
                                cv::Scalar(255, 0, 0));
            }
        }
    }

    void PosePipeline::drawRoi(cv::Mat &output) const {
        const auto p1 = cv::Point(roi.x, roi.y);
        const auto p2 = cv::Point(roi.x + roi.w, roi.y + roi.h);
        cv::Scalar color(0, 255, 0);
        cv::line(output, p1, cv::Point(roi.x + roi.w, roi.y), color, 2);
        cv::line(output, p1, cv::Point(roi.x, roi.y + roi.h), color, 2);
        cv::line(output, p2, cv::Point(roi.x, roi.y + roi.h), color, 2);
        cv::line(output, p2, cv::Point(roi.x + roi.w, roi.y), color, 2);
    }

    std::chrono::nanoseconds PosePipeline::timestamp() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());
    }

    float PosePipeline::getThreshold() const {
        return threshold;
    }

    void PosePipeline::setThreshold(float _threshold) {
        threshold = _threshold;
    }

    void PosePipeline::setFilterWindowSize(int size) {
        f_win_size = size;
        for (auto &filter: filters) {
            filter.setWindowSize(size);
        }
    }

    void PosePipeline::setFilterVelocityScale(float scale) {
        f_v_scale = scale;
        for (auto &filter: filters) {
            filter.setVelocityScale(scale);
        }
    }

    void PosePipeline::setFilterTargetFps(int fps) {
        f_fps = fps;
        for (auto &filter: filters) {
            filter.setTargetFps(fps);
        }
    }

    float PosePipeline::getFilterVelocityScale() const {
        return f_v_scale;
    }

    int PosePipeline::getFilterTargetFps() const {
        return f_fps;
    }

    int PosePipeline::getFilterWindowSize() const {
        return f_win_size;
    }

} // eox