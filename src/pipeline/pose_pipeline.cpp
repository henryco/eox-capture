//
// Created by henryco on 1/10/24.
//

#include "pose_pipeline.h"

namespace eox {

    std::chrono::nanoseconds PosePipeline::timestamp() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());
    }

    void PosePipeline::init() {

        filters.clear();
        filters.reserve(117);
        for (int i = 0; i < 117; i++) {
            filters.emplace_back(30, 0.5f, 30);
        }

        initialized = true;
    }

    int PosePipeline::pass(const cv::Mat &frame) {
        cv::Mat segmentation;
        return pass(frame, segmentation);
    }

    int PosePipeline::pass(const cv::Mat &frame, cv::Mat &segmented) {
        return inference(frame, segmented, nullptr);
    }

    int PosePipeline::pass(const cv::Mat &frame, cv::Mat &segmented, cv::Mat &aux) {
        return inference(frame, segmented, &aux);
    }

    int PosePipeline::inference(const cv::Mat &frame, cv::Mat &segmented, cv::Mat *output) {

        if (!initialized) {
            init();
        }

        cv::Mat source;
        if (prediction) {
            // crop using roi
            log->info("x: {}, y: {}, w: {}, h: {}, fw: {}, fh: {}", roi.x, roi.y, roi.w, roi.h, frame.cols, frame.rows);

//            source = frame(cv::Rect(roi.x, roi.y, roi.w, roi.h));
            roi = {.x = 0, .y = 0, .w = 640, .h = 480};
            source = frame;
        } else {
            // use detector model TODO
            roi = {.x = 0, .y = 0, .w = frame.cols, .h = frame.cols};
            source = frame;
        }

        auto result = pose.inference(source);
        if (result.presence > threshold) {
            prediction = true;

            // temporal filtering (low pass based on velocity)
            const auto now = timestamp();
            for (int i = 0; i < 39; i++) {
                const auto idx = i * 3;
                auto fx = filters.at(idx + 0).filter(now, result.landmarks_norm[i].x);
                auto fy = filters.at(idx + 1).filter(now, result.landmarks_norm[i].y);
                auto fz = filters.at(idx + 2).filter(now, result.landmarks_norm[i].z);

                result.landmarks_norm[i].x = fx;
                result.landmarks_norm[i].y = fy;
                result.landmarks_norm[i].z = fz;
            }

            roi = roiPredictor.forward(
                    {
                            .pose = result,
                            .origin_roi = roi,
                            .origin_w = frame.cols,
                            .origin_h = frame.rows
                    });

            {
                // Segmentation
                cv::Mat segmentation(128, 128, CV_32F, result.segmentation);
                cv::Mat segmentation_mask;
                cv::threshold(segmentation, segmentation_mask, 0.5, 1., cv::THRESH_BINARY);
                cv::resize(segmentation_mask, segmentation_mask, cv::Size(frame.cols, frame.rows));
                segmentation_mask.convertTo(segmentation_mask, CV_32FC1, 255.);
                segmentation_mask.convertTo(segmentation_mask, CV_8UC1);
                cv::bitwise_and(frame, frame, segmented, segmentation_mask);
            }

            // auxiliary (debug) draw
            if (output) {
                segmented.copyTo(*output);
                drawJoints(result.landmarks_norm, *output);
                drawLandmarks(result.landmarks_norm, *output);
                drawRoi(*output);
            }

        } else {
            prediction = false;
        }

        return 0;
    }

    void PosePipeline::drawJoints(const eox::dnn::Landmark landmarks[39], cv::Mat &output) const {
        for (auto bone: eox::dnn::body_joints) {
            const auto &start = landmarks[bone[0]];
            const auto &end = landmarks[bone[1]];
            if (eox::dnn::sigmoid(start.p) > threshold && eox::dnn::sigmoid(end.p) > threshold) {
                cv::Point sp(start.x * output.cols, start.y * output.rows);
                cv::Point ep(end.x * output.cols, end.y * output.rows);
                cv::Scalar color(230, 0, 230);
                cv::line(output, sp, ep, color, 2);
            }
        }
    }

    void PosePipeline::drawLandmarks(const eox::dnn::Landmark landmarks[39], cv::Mat &output) const {
        for (int i = 0; i < 39; i++) {
            const auto point = landmarks[i];
            if (eox::dnn::sigmoid(point.v) > threshold || i > 32) {
                cv::Point circle(point.x * output.cols, point.y * output.rows);
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

    float PosePipeline::getThreshold() const {
        return threshold;
    }

    void PosePipeline::setThreshold(float _threshold) {
        threshold = _threshold;
    }

} // eox