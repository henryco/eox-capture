//
// Created by henryco on 1/12/24.
//

#include "pose_detector.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"

#include <filesystem>
#include <opencv2/imgproc.hpp>

namespace eox::dnn {

    namespace dtc {
        const float *detector_bboxes_1x2254x12(const tflite::Interpreter &interpreter) {
            return interpreter.output_tensor(0)->data.f;
        }

        const float *detector_scores_1x2254x1(const tflite::Interpreter &interpreter) {
            return interpreter.output_tensor(1)->data.f;
        }
    }

    const std::vector<std::string> PoseDetector::outputs = {
            "Identity",   // 441  | 0: [1, 2254, 12]           un-decoded face bboxes location and key-points
            "Identity_1", // 1429 | 4: [1, 2254, 1]            scores of the detected bboxes
    };

    PoseDetector::PoseDetector() {
        if (!std::filesystem::exists(file)) {
            log->error("File: " + file + " does not exists!");
            throw std::runtime_error("File: " + file + " does not exists!");
        }
    }

    PoseDetector::~PoseDetector() {
        if (gpu_delegate) {
            TfLiteGpuDelegateV2Delete(gpu_delegate);
        }
    }

    void PoseDetector::init() {
        if (initialized)
            return;

        log->info("INIT");

        anchors_vec = eox::dnn::ssd::generate_anchors(eox::dnn::ssd::SSDAnchorOptions(
                5,
                0.15,
                0.75,
                in_resolution,
                in_resolution,
                0.5,
                0.5,
                {8, 16, 32, 32, 32},
                {1.0},
                false,
                1.0,
                true
                ));

        model = std::move(tflite::FlatBufferModel::BuildFromFile(file.c_str()));
        if (!model) {
            log->error("Failed to load tflite model");
            throw std::runtime_error("Failed to load tflite model");
        }

        tflite::ops::builtin::BuiltinOpResolver resolver;
        tflite::InterpreterBuilder(*model, resolver)(&interpreter);
        if (!interpreter) {
            log->error("Failed to create tflite interpreter");
            throw std::runtime_error("Failed to create tflite interpreter");
        }

        TfLiteGpuDelegateOptionsV2 options = TfLiteGpuDelegateOptionsV2Default();
        gpu_delegate = TfLiteGpuDelegateV2Create(&options);

        if (interpreter->ModifyGraphWithDelegate(gpu_delegate) != kTfLiteOk) {
            log->error("Failed to modify graph with GPU delegate");
            throw std::runtime_error("Failed to modify graph with GPU delegate");
        }

        if (interpreter->AllocateTensors() != kTfLiteOk) {
            log->error("Failed to allocate tensors for tflite interpreter");
            throw std::runtime_error("Failed to allocate tensors for tflite interpreter");
        }

        int i = 0;
        for (const auto &item: interpreter->outputs()) {
            log->info("T_O: {}, {}, {}", item, i, interpreter->GetOutputName(i));

            auto tensor = interpreter->output_tensor(i);
            log->info("size: {}", tensor->bytes);
            i++;
        }

        initialized = true;
    }

    std::vector<eox::dnn::DetectedRegion> PoseDetector::inference(const float *frame) {
        init();

        auto input = interpreter->input_tensor(0)->data.f;
        std::memcpy(input, frame, in_resolution * in_resolution * 3 * 4); // 224*224*3*4 = 602112

        if (interpreter->Invoke() != kTfLiteOk) {
            log->error("Failed to invoke interpreter");
            throw std::runtime_error("Failed to invoke interpreter");
        }

        return process();
    }

    std::vector<eox::dnn::DetectedRegion> PoseDetector::inference(cv::InputArray &frame) {
        cv::Mat blob = eox::dnn::convert_to_squared_blob(frame.getMat(), in_resolution, true);
        return inference(blob.ptr<float>(0));
    }

    std::vector<eox::dnn::DetectedRegion> PoseDetector::process() {

        const auto bboxes = dtc::detector_bboxes_1x2254x12(*interpreter);
        const auto scores = dtc::detector_scores_1x2254x1(*interpreter);

        std::vector<float> scores_vec;
        std::vector<std::array<float, 12>> bboxes_vec;

        scores_vec.reserve(2254);
        bboxes_vec.reserve(2254);

        for (int i = 0; i < 2254; i++) {
            scores_vec.push_back(scores[i]);

            std::array<float, 12> det_bbox{};
            for (int k = 0; k < 12; k++) {
                det_bbox[k] = bboxes[i * 12 + k];
            }
            bboxes_vec.push_back(det_bbox);
        }

        auto boxes = eox::dnn::ssd::decode_bboxes(
                threshold,
                scores_vec,
                bboxes_vec,
                anchors_vec,
                in_resolution,
                true);

        // TODO fix letterboxes?

        return boxes;
    }

    float PoseDetector::getThreshold() const {
        return threshold;
    }

    void PoseDetector::setThreshold(float _threshold) {
        threshold = _threshold;
    }

} // eox