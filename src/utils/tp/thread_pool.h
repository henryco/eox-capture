//
// Created by henryco on 11/16/23.
//

#ifndef STEREOX_THREAD_POOL_H
#define STEREOX_THREAD_POOL_H


#include <mutex>
#include <queue>
#include <thread>
#include <functional>
#include <future>

class ThreadPool {
public:

    explicit ThreadPool(int size);

    ~ThreadPool();

    void shutdown();

    template<typename T>
    std::future<T> execute(std::function<T()>&& func) {
        std::future<T> future = enqueue(func);
        trigger();
        return future;
    }

private:
    std::queue<std::function<void()>> tasks;
    std::queue<std::thread> threads;
    std::condition_variable flag;
    std::mutex mutex;
    bool stop = false;

    void worker();

    void trigger();

    template<typename T>
    std::future<T> enqueue(std::function<T()>&& func) {
        std::promise<T> promise;
        std::future<T> future = promise.get_future();

        auto lambda = [p = std::move(promise), &func, this]() mutable {
            try {
                p.set_value(func());
            } catch (...) {
                try {
                    p.set_exception(std::current_exception());
                } catch (...) {
                    p.set_exception(std::runtime_error("unknown error during task execution"));
                }
            }
            this->trigger();
        };

        {
            std::unique_lock<std::mutex> lock(mutex);
            tasks.push(std::move(lambda));
            lock.unlock();
        }

        return future;
    }
};


#endif //STEREOX_THREAD_POOL_H
