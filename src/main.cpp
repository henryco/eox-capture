#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include "calibration/ui_calibration.h"
#include "aux/utils/errors/error_reporter.h"
#include "cli.h"
#include "cloud/ui_points_cloud.h"
#include "pose/ui_pose.h"

int main(int argc, char **argv) {

    {
        // init opencl
        cv::ocl::setUseOpenCL(true);

        if (!cv::ocl::useOpenCL()) {
            std::cerr << "OpenCL is not available..." << '\n';
            return -1;
        }
    }

    try {
        const auto configuration = eox::cli::parse(argc, argv);

        int n_argc = 1;
        const auto app = Gtk::Application::create(
                n_argc,
                argv,
                "dev.tindersamurai.stereox"
        );

        std::unique_ptr<eox::xgtk::GtkEoxWindow> window;

        if (configuration.module == "calibration")
            window = std::make_unique<eox::UiCalibration>();

        else if (configuration.module == "stereo")
            window = std::make_unique<eox::UiPointsCloud>();

        else if (configuration.module == "pose")
            window = std::make_unique<eox::UiPose>();

        else return 1;

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
