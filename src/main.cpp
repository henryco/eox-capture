#include <gtkmm.h>
#include "calibration/ui_calibration.h"
#include "utils/errors/error_reporter.h"
#include "cli/cli.h"

int main(int argc, char **argv) {

    try {
        const auto configuration = sex::cli::parse(argc, argv);

        int n_argc = 1;
        const auto app = Gtk::Application::create(
                n_argc,
                argv,
                "dev.tindersamurai.stereox"
        );

        sex::xgtk::GtkSexWindow *window = nullptr;
        if (configuration.module == "calibration")
            window = new UiCalibration();

        if (window == nullptr)
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
