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
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace sex {

    class ThreadPool {
    public:

        explicit ThreadPool(size_t size);

        ThreadPool() = default;

        ~ThreadPool();

        void shutdown();

        template<typename T>
        std::future<T> execute(std::function<T()> func) {
            auto le_promise = std::make_shared<std::promise<T>>();
            auto le_future = le_promise->get_future();

            auto lambda = [p = le_promise, func, this]() mutable {
                try {
                    p->set_value(func());
                } catch (...) {
                    p->set_exception(std::current_exception());
                }
                this->trigger();
            };

            {
                std::unique_lock<std::mutex> lock(mutex);
                tasks.push(std::move(lambda));
                flag.notify_all();
                lock.unlock();
            }

            return le_future;
        }

        void start(size_t size);

    private:
        static inline const auto log =
                spdlog::stdout_color_mt("thread_pool");

        std::queue<std::function<void()>> tasks;
        std::queue<std::thread> threads;
        std::condition_variable flag;
        std::mutex mutex;

        std::atomic<bool> stop = true;

        void worker();

        void trigger();
    };

}

#endif //STEREOX_THREAD_POOL_H
