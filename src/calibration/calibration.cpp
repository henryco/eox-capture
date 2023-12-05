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
    if (preCalibrated.empty() || image_points.size() < 2 || preCalibrated.size() != image_points.size()) {

        // there is no pre-calibrated matrices from configuration,
        // or there is only one camera, OR not every camera is pre-calibrated.
        // In any case we need to calibrate each camera first

        calibrated_solo.reserve(props.size());
        for (const auto &prop: props) {
            const auto &c_id = prop.id;
            const auto result = eox::ocv::calibrate_solo(
                    image_points[c_id],
                    c_id,
                    config.camera[0].width,
                    config.camera[0].height,
                    config.calibration.rows,
                    config.calibration.columns
            );
            log->info("RMS[{}]: {}", c_id, result.rms);
            log->info("MRE[{}]: {}", c_id, result.mre);
            for (const auto &err: result.per_view_errors) {
                log->debug("-> {}", err);
            }
            calibrated_solo.push_back(result);
        }
    } else {

        // there are calibration matrices from configuration file
        // just load it

        calibrated_solo.reserve(preCalibrated.size());
        for (auto [uid, solo]: preCalibrated) {
            log->debug("reading calibration data from file: {}", uid);
            calibrated_solo.push_back(std::move(solo));
        }
    }

    // unpack map of corners to left and right
    std::vector<std::vector<std::vector<cv::Point2f>>> points;
    points.reserve(image_points.size());
    for (const auto &[id, vec]: image_points) {
        points.push_back(vec);
    }

    if (points.size() < 2) {

        // not enough cameras for stereo calibration

        log->debug("single camera, skip stereo calibration");

        std::map<uint, eox::ocv::CalibrationSolo> solo_map;
        solo_map.emplace(calibrated_solo[0].uid, calibrated_solo[0]);

        stereoPackage = {.solo = std::move(solo_map)};

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
    log->info("RMS: {}", stereo_calibration.rms);

    // stereo rectification
    auto stereo_rectification = eox::ocv::rectify_stereo(
            calibrated_solo[0],
            calibrated_solo[1],
            stereo_calibration
    );

    // adapting and saving results
    std::map<uint, eox::ocv::CalibrationSolo> solo_map;
    for (const auto &solo: calibrated_solo) {
        solo_map.emplace(solo.uid, solo);
    }

    stereoPackage = {
            .solo = std::move(solo_map),
            .stereo = std::move(stereo_calibration),
            .rectification = std::move(stereo_rectification)
    };

    update_ui(remains, frames);
}

#pragma clang diagnostic pop