//
// Created by henryco on 11/16/23.
//

#include "thread_pool.h"

void ThreadPool::worker() {
    while (true) {

        {
            std::unique_lock<std::mutex> stopLock(mutex);
            if (stop) {
                stopLock.unlock();
                return;
            }

            flag.wait(stopLock);

            if (stop) {
                stopLock.unlock();
                return;
            }
            stopLock.unlock();
        }

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
    }
}

void ThreadPool::trigger() {
    std::lock_guard<std::mutex> lock(mutex);
    flag.notify_all();
}

void ThreadPool::shutdown() {
    std::lock_guard<std::mutex> lock(mutex);
    stop = true;
    flag.notify_all();
}

ThreadPool::~ThreadPool() {
    shutdown();
    while (!threads.empty()) {
        auto thread = std::move(threads.front());
        threads.pop();
        if (thread.joinable())
            thread.join();
    }
}

ThreadPool::ThreadPool(int size) {
    for (int i = 0; i < size; ++i) {
        threads.emplace(&ThreadPool::worker, this);
    }
}
