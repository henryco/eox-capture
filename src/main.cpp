#include <gtkmm.h>
#include "utils/globals/sex_globals.h"
#include "calibration/ui_calibration.h"
#include "utils/errors/error_reporter.h"
#include "spdlog/spdlog.h"

int main(int argc, char **argv) {

    sex::globals::THREAD_POOL_CORES_MAX = 4;


    // TEMPORAL (move to CLI later)
    const std::map<uint, uint> devices = {
            // ID, INDEX
            {1, 4},
            {2, 2}
    };
    const std::string codec = "MJPG";
    const int width = 640;
    const int height = 480;
    const int fps = 30;
    const bool homogeneous = true;
    const bool fast = false;
    const int api = cv::CAP_V4L2;
    const int buffer = 2;
    // TEMPORAL (move to CLI later)


    std::vector<sex::data::camera_properties> props;
    props.reserve(devices.size());
    for (const auto &[id, index]: devices) {
        props.push_back({
                                .id = id,
                                .index = index,
                                .width = width,
                                .height = height,
                                .fps = fps,
                                .buffer = buffer,
                                .codec = {codec[0],
                                          codec[1],
                                          codec[2],
                                          codec[3]},
                                .fast = fast,
                                .homogeneous = homogeneous,
                                .api = api
                        });
    }


    try {

//        spdlog::set_level(spdlog::level::debug);
        spdlog::set_level(spdlog::level::info);

        const auto app = Gtk::Application::create(
                argc,
                argv,
                "dev.tindersamurai.stereox"
        );

        sex::xgtk::GtkSexWindow* window = new UiCalibration();
        window->init({ .camera = props });
        window->show();
        return app->run(*window);

    } catch (const std::exception &e) {
        ErrorReporter::printStackTrace(e);
        ErrorReporter::dumpError(e);
        return EXIT_FAILURE;
    } catch (...) {
        ErrorReporter::printStackTrace();
        ErrorReporter::dumpError();
        return EXIT_FAILURE;
    }
}
