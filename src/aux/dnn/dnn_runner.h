//
// Created by henryco on 1/18/24.
//

#ifndef STEREOX_DNN_RUNNER_H
#define STEREOX_DNN_RUNNER_H

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <filesystem>
//#include <spdlog/logger.h>
//#include <spdlog/sinks/stdout_color_sinks.h>
#include <opencv2/core/mat.hpp>
#include "tensorflow/lite/delegates/gpu/delegate_options.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"

namespace eox::dnn {

    template <typename T>
    class DnnRunner {
//        static inline const auto log =
//                spdlog::stdout_color_mt("dnn_runner");

    protected:
        std::unique_ptr<tflite::FlatBufferModel> model;
        std::unique_ptr<tflite::Interpreter> interpreter;
        TfLiteDelegate* gpu_delegate = nullptr;
        bool initialized = false;

        virtual std::string get_model_file() = 0;

        void input(int index, const float *frame_ptr, size_t size) {
            auto input = interpreter->input_tensor(index)->data.f;
            std::memcpy(input, frame_ptr, size); // 256*256*3*4 = 786432
        }

        void invoke() {
            if (interpreter->Invoke() != kTfLiteOk) {
//                log->error("Failed to invoke interpreter");
                throw std::runtime_error("Failed to invoke interpreter");
            }
        }

        virtual void initialize() {

        }

    public:

        /**
         * @param frame pointer to row-oriented 1D array representation of RGB image
         */
        virtual T inference(const float *frame) = 0;

        virtual ~DnnRunner() {
            if (gpu_delegate) {
                TfLiteGpuDelegateV2Delete(gpu_delegate);
            }
        }

        void reset() {
            if (gpu_delegate) {
                TfLiteGpuDelegateV2Delete(gpu_delegate);
            }
            initialized = false;
        }

        void init() {
            if (initialized)
                return;

            initialize();

            if (!std::filesystem::exists(get_model_file())) {
//                log->error("File: " + get_model_file() + " does not exists!");
                throw std::runtime_error("File: " + get_model_file() + " does not exists!");
            }

            model = std::move(tflite::FlatBufferModel::BuildFromFile(get_model_file().c_str()));
            if (!model) {
//                log->error("Failed to load tflite model");
                throw std::runtime_error("Failed to load tflite model");
            }

            tflite::ops::builtin::BuiltinOpResolver resolver;
            tflite::InterpreterBuilder(*model, resolver)(&interpreter);
            if (!interpreter) {
//                log->error("Failed to create tflite interpreter");
                throw std::runtime_error("Failed to create tflite interpreter");
            }

            TfLiteGpuDelegateOptionsV2 options = TfLiteGpuDelegateOptionsV2Default();
            gpu_delegate = TfLiteGpuDelegateV2Create(&options);

            if (interpreter->ModifyGraphWithDelegate(gpu_delegate) != kTfLiteOk) {
//                log->error("Failed to modify graph with GPU delegate");
                throw std::runtime_error("Failed to modify graph with GPU delegate");
            }

            if (interpreter->AllocateTensors() != kTfLiteOk) {
//                log->error("Failed to allocate tensors for tflite interpreter");
                throw std::runtime_error("Failed to allocate tensors for tflite interpreter");
            }

            initialized = true;
        }
    };

} // eox

#endif //STEREOX_DNN_RUNNER_H
