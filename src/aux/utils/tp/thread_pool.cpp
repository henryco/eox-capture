//
// Created by henryco on 11/16/23.
//

#include <iostream>
#include "thread_pool.h"
#include "../globals/eox_globals.h"

namespace eox::util {

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
        size_t n_cores = std::clamp(size,
                                    std::size_t(1),
                                    eox::globals::THREAD_POOL_CORES_MAX);
        log->debug("start, requested: {}, given: {}", size, n_cores);
        shutdown();
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (int i = 0; i < n_cores; ++i) {
                threads.emplace(&ThreadPool::worker, this);
            }
            stop = false;
            flag.notify_all();
        }
        log->debug("started");
    }


    std::future<void> ThreadPool::execute(std::function<void()> func) {
        auto le_promise = std::make_shared<std::promise<void>>();
        auto le_future = le_promise->get_future();

        auto lambda = [p = le_promise, func = std::move(func), this]() mutable {
            try {
                func();
                p->set_value();
            } catch (...) {
                p->set_exception(std::current_exception());
            }
            this->trigger();
        };

        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.emplace(std::move(lambda));
            flag.notify_all();
        }

        return le_future;
    }

}