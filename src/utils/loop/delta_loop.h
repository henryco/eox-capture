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

namespace sex {

    class DeltaLoop {
    private:
        const std::chrono::nanoseconds frame;
        std::unique_ptr<std::thread> thread;
        std::function<void(float)> runnable;

        std::atomic<bool> alive = false;
        std::condition_variable flag;
        std::mutex mutex;

    public:
        explicit DeltaLoop(int fps = 60);

        explicit DeltaLoop(std::function<void(float)> runnable,  int fps = 60);

        ~DeltaLoop();

        void start();

        void stop();

        void setFunc(std::function<void(float)> _runnable);

    protected:
        void worker();

    };

} // sx

#endif //STEREOX_DELTA_LOOP_H
