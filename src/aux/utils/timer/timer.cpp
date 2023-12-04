//
// Created by henryco on 12/2/23.
//

#include "timer.h"

namespace eox::utils {

    Timer::Timer(): active(false) {}

    Timer::Timer(int delay) :
    delay(std::chrono::milliseconds(delay)),
    active(false) {}

    int Timer::get_delay() {
        return (int) delay.count();
    }

    int Timer::tick(const std::function<void()>& callback) {
        if (!active)
            return 0;
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());
        const auto diff = now - point;
        if (diff < delay) {
            return (int) (delay - diff).count();
        }
        callback();
        reset();
        return 0;
    }

    void Timer::start() {
        active = true;
    }

    void Timer::stop() {
        reset();
        active = false;
    }

    void Timer::reset() {
        point = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());
    }

    void Timer::set_delay(int _delay) {
        delay = std::chrono::milliseconds(_delay);
    }


} // utils