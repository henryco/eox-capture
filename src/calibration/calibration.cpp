//
// Created by henryco on 12/3/23.
//

#include "ui_calibration.h"

void UiCalibration::update(float delta, float latency, float _fps) {
//    log->debug("update: {}, late: {}, fps: {}", delta, latency, _fps);
    this->FPS = _fps;

    auto captured = camera.capture();
    if (captured.empty()) {
        log->debug("skip");
        return;
    }

    if (!active) {
        glImage.setFrames(captured);
        refresh();
        return;
    }

    std::vector<std::future<eox::ocv::Squares>> futures;
    futures.reserve(captured.size());
    for (const auto &frame: captured) {
        futures.push_back(executor->execute<eox::ocv::Squares>([&frame, this]() {
            return eox::ocv::find_squares(
                    frame,
                    config.calibration.columns,
                    config.calibration.rows,
                    config.calibration.quality);
        }));
    }

    int found = 0;
    std::vector<cv::Mat> frames;
    frames.reserve(futures.size());
    for (auto &future: futures) {
        const auto squares = future.get();
        frames.push_back(squares.result);
        found += squares.found;
    }

    if (found != futures.size()) {
        timer.reset();
        progress = 0;
    } else {
        const auto remains = timer.tick([]() {
            log->debug("TICKED");
            // TODO LOGIC
        });
        log->debug("REMAINS: {}", remains / 1000.);

        const auto fraction = (double) remains / timer.get_delay();
        if (std::isfinite(fraction))
            progress = fraction;
        else progress = 0;
    }

    glImage.setFrames(frames);
    refresh();
}