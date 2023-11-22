//
// Created by henryco on 11/21/23.
//

#ifndef STEREOX_DELTA_LOOP_H
#define STEREOX_DELTA_LOOP_H

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace sex {

    class DeltaLoop {
        static inline const auto log =
                spdlog::stdout_color_mt("delta_loop");

    private:
        const std::chrono::nanoseconds frame;
        std::unique_ptr<std::thread> thread;
        std::function<void(float, float)> runnable;

        std::atomic<bool> alive = false;
        std::condition_variable flag;
        std::mutex mutex;

    public:
        explicit DeltaLoop(int fps = 60);

        explicit DeltaLoop(std::function<void(float, float)> runnable,  int fps = 60);

        ~DeltaLoop();

        void start();

        void stop();

        void setFunc(std::function<void(float, float)> _runnable);

    protected:
        void worker();

    };

} // sx

#endif //STEREOX_DELTA_LOOP_H
