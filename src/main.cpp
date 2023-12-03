#include <gtkmm.h>
#include "calibration/ui_calibration.h"
#include "aux/utils/errors/error_reporter.h"
#include "cli.h"

int main(int argc, char **argv) {

    try {
        const auto configuration = sex::cli::parse(argc, argv);

        int n_argc = 1;
        const auto app = Gtk::Application::create(
                n_argc,
                argv,
                "dev.tindersamurai.stereox"
        );

        std::unique_ptr<sex::xgtk::GtkSexWindow> window;
        if (configuration.module == "calibration")
            window = std::make_unique<UiCalibration>();

        if (!window)
            return 1;

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
