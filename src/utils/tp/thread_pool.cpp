//
// Created by henryco on 11/16/23.
//

#include <iostream>
#include "thread_pool.h"

namespace sex {

    void ThreadPool::worker() {
        while (true) {

            while (true) {
                std::unique_lock<std::mutex> stateLock(mutex);

                if (stop) {
                    stateLock.unlock();
                    return;
                }

                if (tasks.empty()) {
                    stateLock.unlock();
                    break;
                }

                auto task = std::move(tasks.front());
                tasks.pop();
                stateLock.unlock();

                if (task)
                    task();
            }

            {
                std::unique_lock<std::mutex> stopLock(mutex);
                if (stop) {
                    stopLock.unlock();
                    return;
                }

                if (!tasks.empty()) {
                    stopLock.unlock();
                    continue;
                }

                flag.wait(stopLock);

                if (stop) {
                    stopLock.unlock();
                    return;
                }
                stopLock.unlock();
            }
        }
    }

    void ThreadPool::trigger() {
        std::lock_guard<std::mutex> lock(mutex);
        flag.notify_all();
    }

    void ThreadPool::shutdown() {
        log->debug("shutdown thread pool");

        {
            std::lock_guard<std::mutex> lock(mutex);
            stop = true;
            flag.notify_all();
        }

        while (!threads.empty()) {

            log->debug("freeing pool");

            auto thread = std::move(threads.front());
            threads.pop();

            log->debug("pool wait");

            if (thread.joinable()) {
                thread.join();
            }
        }

        log->debug("pool freed");
    }

    ThreadPool::~ThreadPool() {
        shutdown();
    }

    ThreadPool::ThreadPool(size_t size) {
        start(size);
    }

    void ThreadPool::start(size_t size) {
        log->debug("start: {}", size);
        shutdown();
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (int i = 0; i < size; ++i) {
                threads.emplace(&ThreadPool::worker, this);
            }
            stop = false;
            flag.notify_all();
        }
        log->debug("started");
    }

}