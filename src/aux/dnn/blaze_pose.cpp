//
// Created by henryco on 12/29/23.
//

#include "blaze_pose.h"

#include "tensorflow/lite/delegates/gpu/delegate.h"

namespace eox::dnn {

    double sigmoid(double x) {
        return 1.0 / (1.0 + exp(-x));
    }

    const float *lm_3d_1x195(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(0)->data.f;
    }

    const float *lm_world_1x117(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(1)->data.f;
    }

    const float *heatmap_1x64x64x39(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(2)->data.f;
    }

    const float *segmentation_1x128x128x1(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(3)->data.f;
    }

    const float *pose_flag_1x1(const tflite::Interpreter &interpreter) {
        return interpreter.output_tensor(4)->data.f;
    }

    const std::string BlazePose::file = "./../models/blazepose_heavy_float32.tflite";
    const std::vector<std::string> BlazePose::outputs = {
            "Identity:0",   // 598 | 0: [1, 195]           landmarks 3d
            "Identity_4:0", // 600 | 1: [1, 117]           world 3d
            "Identity_3:0", // 481 | 2: [1, 64, 64, 39]    heatmap
            "Identity_2:0", // 608 | 3: [1, 128, 128, 1]   segmentation
            "Identity_1:0", // 603 | 4: [1, 1]             pose flag (score)
    };

    BlazePose::BlazePose() {
        if (!std::filesystem::exists(file)) {
            log->error("File: " + file + " does not exists!");
            throw std::runtime_error("File: " + file + " does not exists!");
        }
    }

    BlazePose::~BlazePose() {
        if (gpu_delegate) {
            TfLiteGpuDelegateV2Delete(gpu_delegate);
        }
    }

    void BlazePose::init() {
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

    void BlazePose::forward(cv::InputArray &frame) {
        cv::Mat blob;

        {
            cv::resize(frame.getMat(), blob, cv::Size(256, 256));
            cv::cvtColor(blob, blob, cv::COLOR_BGR2RGB);
            blob.convertTo(blob, CV_32FC3, 1.0 / 255.);
        }

        // [1, 3, 256, 256]
        inference(blob.ptr<float>(0));
    }

    void BlazePose::inference(const float *frame) {
        init();

        auto input = interpreter->input_tensor(0)->data.f;
        std::memcpy(input, frame, 786432); // 256*256*3*4 = 786432

        if (interpreter->Invoke() != kTfLiteOk) {
            log->error("Failed to invoke interpreter");
            throw std::runtime_error("Failed to invoke interpreter");
        }

        const auto presence = *pose_flag_1x1(*interpreter);
        log->info("POSE: {}", presence);
    }

} // eox