//
// Created by henryco on 1/12/24.
//

#include "pose_detector.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"

#include <filesystem>
#include <opencv2/imgproc.hpp>

namespace eox::dnn {

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
            log->info("type: {}", tensor->bytes);
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
        cv::Mat blob;
        cv::Mat ref = frame.getMat();
        {
            // TODO FIXME: add letterboxes
            if (ref.cols != in_resolution || ref.rows != in_resolution) {
                cv::resize(ref, blob, cv::Size(in_resolution, in_resolution));
            } else {
                ref.copyTo(blob);
            }
            cv::cvtColor(blob, blob, cv::COLOR_BGR2RGB);
            blob.convertTo(blob, CV_32FC3, 1.0 / 255.);
        }

        return inference(blob.ptr<float>(0));
    }

    std::vector<eox::dnn::DetectedRegion> PoseDetector::process() {
        // TODO
        return {};
    }

} // eox