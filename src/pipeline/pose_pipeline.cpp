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
        constexpr float MARGIN = 30;
        constexpr float FIX_X = 0;
        constexpr float FIX_Y = 10;

        if (!initialized) {
            init();
        }

        PosePipelineOutput output;
        cv::Mat source;

        if (prediction) {
            // crop using roi
            roi = eox::dnn::clamp_roi(roi, frame.cols, frame.rows);
            source = frame(cv::Rect(roi.x, roi.y, roi.w, roi.h));
        }

        if (!prediction) {
            // using pose detector
            auto detections = detector.inference(frame);

            if (detections.empty() || detections[0].score < threshold_detector) {
                output.present = false;
                output.score = 0;

                if (debug)
                    frame.copyTo(*debug);

                return output;
            }

            auto &detected = detections[0];

            auto &body = detected.body;
            body.x *= frame.cols;
            body.y *= frame.rows;
            body.w *= frame.cols;
            body.h *= frame.rows;

            body.x += FIX_X - (MARGIN / 2.f);
            body.y += FIX_Y - (MARGIN / 2.f);
            body.w += (MARGIN / 2.f);
            body.h += (MARGIN / 2.f);

            body.c.x *= frame.cols;
            body.c.y *= frame.rows;
            body.e.x *= frame.cols;
            body.e.y *= frame.rows;

            body.c.x += FIX_X - (MARGIN / 2.f);
            body.c.y += FIX_Y - (MARGIN / 2.f);
            body.e.x += FIX_X - (MARGIN / 2.f);
            body.e.y += FIX_Y - (MARGIN / 2.f);

            auto &face = detected.face;
            face.x *= frame.cols;
            face.y *= frame.rows;
            face.w *= frame.cols;
            face.h *= frame.rows;

            roi = eox::dnn::clamp_roi(body, frame.cols, frame.rows);
            source = frame(cv::Rect(roi.x, roi.y, roi.w, roi.h));

            for (auto &filter: filters) {
                filter.reset();
            }
        }

        auto result = pose.inference(source);
        const auto now = timestamp();

        if (result.score > threshold_pose) {
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
                drawLandmarks(landmarks, result.landmarks_3d,*debug);
                drawRoi(*debug);
            }

            // predict new roi
            roi = roiPredictor
                    .setMargin(MARGIN)
                    .setFixX(FIX_X)
                    .setFixY(FIX_Y)
                    .forward(eox::dnn::roiFromPoseLandmarks39(landmarks));
            prediction = true;

            // output
            {
                for (int i = 0; i < 39; i++) {
                    output.ws_landmarks[i] = result.landmarks_3d[i];
                    output.landmarks[i] = landmarks[i];
                }

                for (int i = 0; i < 128 * 128; i++)
                    output.segmentation[i] = result.segmentation[i];

                output.score = result.score;
                output.present = true;
            }

        } else {

            // retry but without prediction
            if (prediction) {
                prediction = false;
                return inference(frame, segmented, debug);
            }

            // still nothing
            if (debug) {
                frame.copyTo(*debug);
                drawRoi(*debug);
            }
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
            if (eox::dnn::sigmoid(start.p) > threshold_presence && eox::dnn::sigmoid(end.p) > threshold_presence) {
                cv::Point sp(start.x, start.y);
                cv::Point ep(end.x, end.y);
                cv::Scalar color(230, 0, 230);
                cv::line(output, sp, ep, color, 2);
            }
        }
    }

    void PosePipeline::drawLandmarks(const eox::dnn::Landmark landmarks[39], const eox::dnn::Coord3d ws3d[39], cv::Mat &output) const {
        for (int i = 0; i < 39; i++) {
            const auto point = landmarks[i];
            const auto visibility = eox::dnn::sigmoid(point.v);
            const auto presence = eox::dnn::sigmoid(point.p);
            if (presence > threshold_presence || i > 32) {
                cv::Point circle(point.x, point.y);
                cv::Scalar color(255 * (1.f - visibility), 255 * visibility, 0);
                cv::circle(output, circle, 2, color, 3);

                if (i <= 32) {
                    cv::putText(output, std::to_string(visibility), cv::Point(circle.x - 10, circle.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, 0.7,
                                cv::Scalar(0, 0, 255), 2);
                }

                if (i == 25) {
                    cv::putText(output,
                                std::to_string(point.x / output.cols) + ", " +
                                std::to_string(point.y / output.rows) + ", " +
                                std::to_string(point.z),
                                cv::Point(40, 40),
                                cv::FONT_HERSHEY_SIMPLEX, 0.7,
                                cv::Scalar(0, 0, 255), 2);

                    cv::putText(output,
                                std::to_string(ws3d[i].x) + ", " +
                                std::to_string(ws3d[i].y) + ", " +
                                std::to_string(ws3d[i].z),
                                cv::Point(40, 80),
                                cv::FONT_HERSHEY_SIMPLEX, 0.7,
                                cv::Scalar(0, 0, 255), 2);
                }
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

        cv::Point mid(roi.c.x, roi.c.y);
        cv::Point end(roi.e.x, roi.e.y);
        cv::Scalar pc(0, 255, 255);
        cv::circle(output, mid, 3, pc, 3);
        cv::circle(output, end, 3, pc, 3);

        cv::putText(output,
                    std::to_string(mid.x) + ", " +std::to_string(mid.y),
                    cv::Point(40, 120),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7,
                    cv::Scalar(0, 0, 255), 2);

        cv::putText(output,
                    std::to_string(end.x) + ", " +std::to_string(end.y),
                    cv::Point(40, 160),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7,
                    cv::Scalar(0, 0, 255), 2);
    }

    std::chrono::nanoseconds PosePipeline::timestamp() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());
    }

    float PosePipeline::getPoseThreshold() const {
        return threshold_pose;
    }

    void PosePipeline::setPoseThreshold(float threshold) {
        threshold_pose = threshold;
    }

    void PosePipeline::setDetectorThreshold(float threshold) {
        threshold_detector = threshold;
    }

    float PosePipeline::getDetectorThreshold() const {
        return threshold_detector;
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

    void PosePipeline::setPresenceThreshold(float threshold) {
        threshold_presence = threshold;
    }

    float PosePipeline::getPresenceThreshold() const {
        return threshold_presence;
    }

} // eox