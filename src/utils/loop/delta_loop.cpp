//
// Created by henryco on 11/21/23.
//

#include "delta_loop.h"

#include <memory>
#include <utility>
#include <iostream>

namespace sex {

    DeltaLoop::DeltaLoop(const int fps)
    : frame(((long long) ((long long) 1000000000) / fps)) {}

    DeltaLoop::DeltaLoop(std::function<void(float, float)> runnable, const int fps)
    : DeltaLoop(fps) {
        this->runnable = std::move(runnable);
        start();
    }

    DeltaLoop::~DeltaLoop() {
        stop();
    }

    void DeltaLoop::setFunc(std::function<void(float, float)> _runnable) {
        this->runnable = std::move(_runnable);
    }

    void DeltaLoop::worker() {

        auto loop_pre = std::chrono::high_resolution_clock::now();
        auto loop_post = std::chrono::high_resolution_clock::now();
        auto drift = std::chrono::nanoseconds(0);

        while (true) {
            const auto start = std::chrono::high_resolution_clock::now();

            {
                std::unique_lock<std::mutex> lock(mutex);
                if (!alive) {
                    lock.unlock();
                    return;
                }
                lock.unlock();
            }

            auto loop_end = std::chrono::high_resolution_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(loop_end - loop_post);
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(loop_end - loop_pre);
            auto late = duration - frame;

            loop_pre = std::chrono::high_resolution_clock::now();

            runnable(((float) delta.count()) / 1000000, ((float) late.count()) / 1000000);

            loop_post = std::chrono::high_resolution_clock::now();

            if (late.count() > 0)
                continue;

            {
                std::unique_lock<std::mutex> lock(mutex);
                if (!alive) {
                    lock.unlock();
                    return;
                }

                const auto end = std::chrono::high_resolution_clock::now();
                const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                const auto sleep = std::chrono::duration_cast<std::chrono::nanoseconds>(frame - elapsed);
                const auto should = sleep + std::chrono::high_resolution_clock::now();
                const auto total = sleep + drift;

                if (total.count() < 0) {
                    lock.unlock();
                    continue;
                }

                flag.wait_for(lock, total);
                lock.unlock();

                drift += should - (std::chrono::high_resolution_clock::now());
            }
        }
    }

    void DeltaLoop::start() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (alive) {
                lock.unlock();
                return;
            }
            alive = true;
            lock.unlock();
        }
        thread = std::make_unique<std::thread>(&DeltaLoop::worker, this);
    }

    void DeltaLoop::stop() {
        log->debug("stop");

        {
            std::lock_guard<std::mutex> lock(mutex);
            alive = false;
            flag.notify_all();
        }

        if (thread->joinable()) {

            log->debug("wait join");

            thread->join();
        }

        log->debug("stopped");
    }

} // sex
