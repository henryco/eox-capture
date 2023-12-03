//
// Created by henryco on 12/2/23.
//

#ifndef STEREOX_TIMER_H
#define STEREOX_TIMER_H

#include <chrono>
#include <functional>

namespace eox::utils {
    class Timer {
    private:
        std::chrono::milliseconds point{};
        std::chrono::milliseconds delay{};
        bool active;

    public:
        Timer();

        explicit Timer(int delay);

        int tick(const std::function<void()>& callback);

        void start();

        void reset();

        void stop();

        void set_delay(int delay);

        int get_delay();
    };
} // utils

#endif //STEREOX_TIMER_H
