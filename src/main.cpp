#include <opencv2/core.hpp>
#include <gtkmm.h>
#include "calibration/ui_calibration.h"
#include "utils/errors/error_reporter.h"
#include "spdlog/spdlog.h"

int main(int argc, char **argv) {

    try {
        spdlog::set_level(spdlog::level::debug);

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
