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

namespace eox::util {


    /**
     * @class DeltaLoop
     * @brief A class that provides a delta time loop for running at a specific frame rate.
     */

    class DeltaLoop {
        static inline const auto log =
                spdlog::stdout_color_mt("delta_loop");

    private:
        std::chrono::nanoseconds frame;
        std::unique_ptr<std::thread> thread;
        std::function<void(float, float, float)> runnable;

        std::atomic<bool> alive = false;
        std::condition_variable flag;
        std::mutex mutex;

    public:

        /**
         * @class DeltaLoop
         *
         * @brief The DeltaLoop class provides a simple way to create a main loop to update with a specified frame rate.
         *
         * By default, the DeltaLoop constructor initializes the frame rate to 0 frames per second (fps), which means the loop
         * will run as fast as possible. However, you can provide a specific value for fps to control the maximum frame rate.
         *
         * Example Usage:
         * @code
         * DeltaLoop loop(60); // Creates a DeltaLoop instance with a frame rate of 60 fps
         */

        explicit DeltaLoop(int fps = 0);

        /**
         * @class DeltaLoop
         *
         * @brief Executes a given function repeatedly at a specified frame rate (fps),
         *        or as fast as possible if no frame rate is specified.
         *
         * The DeltaLoop class allows you to execute a given function repeatedly at a specified
         * frame rate (fps). It provides a simple way to control the execution speed of your code,
         * ensuring that it runs at a consistent rate, regardless of the performance of the system.
         *
         * If no frame rate is specified, the code will execute as fast as possible.
         *
         * Usage example:
         * @code{.cpp}
         * // Define a function to be executed repeatedly
         * void myFunction(float deltaTime, float latency, float fps)
         * {
         *     // Do something based on the time difference between update calls
         *     // and the total elapsed time
         *     // ...
         * }
         *
         * // Create a DeltaLoop object and pass the function to be executed
         * DeltaLoop myDeltaLoop(myFunction, 60);
         *
         * // Stop the delta loop when no longer needed
         * myDeltaLoop.stop();
         * @endcode
         */

        explicit DeltaLoop(std::function<void(float, float, float)> runnable, int fps = 0);

        ~DeltaLoop();

        void start();

        void stop();

        void setFunc(std::function<void(float, float, float)> _runnable);

        void setFps(int fps);

        float getFrameLength();

    protected:

        /**
        * @brief Worker function for the DeltaLoop class.
        *
        * This function performs the main computation of the DeltaLoop algorithm.
        * It is responsible for repeatedly executing the user-provided computation
        * function until the convergence condition is met.
        */

        void worker();

    };

} // eox

#endif //STEREOX_DELTA_LOOP_H
