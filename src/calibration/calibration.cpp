//
// Created by henryco on 12/3/23.
//

#include "ui_calibration.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"

void UiCalibration::update(float delta, float latency, float _fps) {
    this->FPS = _fps;

    auto captured = camera.capture();
    if (captured.empty()) {
        // nothing captured at all
        log->debug("skip");
        return;
    }

    if (!active) {
        // calibration off, just return captured frames
        update_ui(0, captured);
        return;
    }

    // find squares on checkerboard (async)
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

    std::vector<eox::ocv::Squares> squares;
    squares.reserve(futures.size());

    // get results of square finding tasks
    for (auto &future: futures) {
        const auto s = future.get();
        frames.push_back(s.result);
        squares.push_back(s);
        found += s.found;
    }

    if (found != futures.size()) {
        // not every frame has squares found
        timer.reset();
        update_ui(0, frames);
        return;
    }

    const auto &props = camera.getProperties();
    const auto remains = timer.tick([this, &squares, &props]() {
        // SNAP! save found square coordinates for each capture
        for (int i = 0; i < squares.size(); i++) {
            const uint c_id = props[i].id;
            auto &s = squares[i];
            auto &points = image_points[c_id];
            points.push_back(s.corners);
        }
        cap += 1;
    });

    if (cap < config.calibration.number) {
        // need more squares
        update_ui(remains, frames);
        return;
    }

    // Now we have all data we need
    active = false;
    timer.reset();

    // first calibrate each camera solo
    std::vector<eox::ocv::CalibrationSolo> calibrated_solo;
    calibrated_solo.reserve(props.size());
    for (const auto &prop: props) {
        const auto &c_id = prop.id;
        const auto result = eox::ocv::calibrate_solo(
                image_points[c_id],
                config.camera[0].width,
                config.camera[0].height,
                config.calibration.rows,
                config.calibration.columns
        );
        log->debug("ERROR[{}]: {}", c_id, result.rms);
        for (const auto &err: result.per_view_errors) {
            log->debug("-> {}", err);
        }
        calibrated_solo.push_back(result);
    }

    // unpack map of corners to left and right
    std::vector<std::vector<std::vector<cv::Point2f>>> points;
    points.reserve(image_points.size());
    for (const auto &[id, vec]: image_points) {
        points.push_back(vec);
    }

    if (points.size() < 2) {
        // not enough cameras to stereo calibrate
        log->warn("Cannot stereo calibrate single camera");
        update_ui(remains, frames);
        return;
    }

    // calibrate stereo
    auto stereo_calibration = eox::ocv::calibrate_stereo_pair(
            points[0],
            calibrated_solo[0],
            points[1],
            calibrated_solo[1],
            config.calibration.rows,
            config.calibration.columns
    );

    // stereo rectification
    auto stereo_rectification = eox::ocv::rectify_stereo(
            calibrated_solo[0],
            calibrated_solo[1],
            stereo_calibration
    );

    // TODO save results

    update_ui(remains, frames);
}

#pragma clang diagnostic pop