#include <gtkmm.h>
#include "utils/globals/sex_globals.h"
#include "calibration/ui_calibration.h"
#include "utils/errors/error_reporter.h"
#include "spdlog/spdlog.h"

int main(int argc, char **argv) {

    sex::globals::THREAD_POOL_CORES_MAX = 4;

    try {
//        spdlog::set_level(spdlog::level::debug);
        spdlog::set_level(spdlog::level::info);

        const auto app = Gtk::Application::create(
                argc,
                argv,
                "dev.tindersamurai.stereox"
        );

        UiCalibration calibration;
        calibration.init();
        calibration.show();
        return app->run(calibration);

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
