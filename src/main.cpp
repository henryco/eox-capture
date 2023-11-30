#include <gtkmm.h>
#include "calibration/ui_calibration.h"
#include "utils/errors/error_reporter.h"
#include "cli/cli.h"

int main(int argc, char **argv) {

    const auto configuration = sex::cli::parse(argc, argv);

    try {
        const auto app = Gtk::Application::create(
                argc,
                argv,
                "dev.tindersamurai.stereox"
        );

        sex::xgtk::GtkSexWindow *window = new UiCalibration();
        window->init(configuration);
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
