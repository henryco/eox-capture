//
// Created by henryco on 1/10/24.
//

#ifndef STEREOX_POSE_PIPELINE_H
#define STEREOX_POSE_PIPELINE_H

#include <vector>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../aux/sig/velocity_filter.h"
#include "../aux/dnn/blaze_pose.h"
#include "../aux/dnn/roi/pose_roi.h"
#include "../aux/dnn/pose_detector.h"

namespace eox {

    using PosePipelineOutput = struct {

        /**
         * pose landmarks in frame's coordinate system
         */
        eox::dnn::Landmark landmarks[39];

        /**
         * pose landmarks in world space
         */
        eox::dnn::Coord3d ws_landmarks[39];

        /**
         * segmentation array
         */
        float segmentation[128 * 128];

        /**
         * presence flag
         */
        bool present;

        /**
         * presence score
         */
        float score;
    };

    class PosePipeline {

        static inline const auto log =
                spdlog::stdout_color_mt("pose_pipeline");

    private:
        std::vector<eox::sig::VelocityFilter> filters;
        eox::dnn::PoseRoi roiPredictor;
        eox::dnn::PoseDetector detector;
        eox::dnn::BlazePose pose;

        bool prediction = false;
        eox::dnn::RoI roi;

        bool initialized = false;

        float threshold_presence = 0.5;
        float threshold_detector = 0.5;
        float threshold_pose = 0.99;

        float f_v_scale = 0.01;
        int f_win_size = 10;
        int f_fps = 30;

    public:
        void init();

        PosePipelineOutput pass(const cv::Mat &frame);

        PosePipelineOutput pass(const cv::Mat &frame, cv::Mat &segmented);

        PosePipelineOutput pass(const cv::Mat &frame, cv::Mat &segmented, cv::Mat &debug);

        void setPresenceThreshold(float threshold);

        void setPoseThreshold(float threshold);

        void setDetectorThreshold(float threshold);

        void setFilterWindowSize(int size);

        void setFilterVelocityScale(float scale);

        void setFilterTargetFps(int fps);

        [[nodiscard]] float getFilterVelocityScale() const;

        [[nodiscard]] int getFilterWindowSize() const;

        [[nodiscard]] int getFilterTargetFps() const;

        [[nodiscard]] float getPoseThreshold() const;

        [[nodiscard]] float getDetectorThreshold() const;

        [[nodiscard]] float getPresenceThreshold() const;

    protected:
        [[nodiscard]] PosePipelineOutput inference(const cv::Mat &frame, cv::Mat &segmented, cv::Mat *debug);

        void performSegmentation(float segmentation_array[128 * 128], const cv::Mat &frame, cv::Mat &out) const;

        void drawJoints(const eox::dnn::Landmark landmarks[39], cv::Mat &output) const;

        void drawLandmarks(const eox::dnn::Landmark landmarks[39], const eox::dnn::Coord3d ws3d[39], cv::Mat &output) const;

        void drawRoi(cv::Mat &output) const;

        [[nodiscard]] std::chrono::nanoseconds timestamp() const;
    };

} // eox

#endif //STEREOX_POSE_PIPELINE_H
