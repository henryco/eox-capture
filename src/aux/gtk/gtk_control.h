//
// Created by henryco on 12/9/23.
//

#ifndef STEREOX_GTK_CONTROL_H
#define STEREOX_GTK_CONTROL_H


#include <gtkmm/box.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/scale.h>

namespace eox::gtk {

    class GtkControl : public Gtk::Box {
        static inline const auto log =
                spdlog::stdout_color_mt("gtk_control");

    private:
        // preventing memory leak for dynamically allocated u_widgets
        std::vector<std::unique_ptr<Gtk::Widget>> controls;

        std::unique_ptr<Gtk::SpinButton> entry;
        std::unique_ptr<Gtk::Scale> scale;

        std::function<double(double value)> callback;
        sigc::connection debounce_connection;

        std::string label;
        bool programmatic_change = false;
        double def_value, min_value, max_value, value, step;

    public:
        static inline const uint DELAY_MS = 500;

        GtkControl(
                std::function<double(double value)> callback,
                std::string label,
                double value,
                double step = 1,
                double def_value = 0,
                double min_value = 0,
                double max_value = 8192);

        GtkControl(
                std::string label,
                double value,
                double step = 1,
                double def_value = 0,
                double min_value = 0,
                double max_value = 8192);

        ~GtkControl() override = default;

        GtkControl& setCallback(std::function<double(double value)> callback);

        void reset();

        void digits(int num);
    };

} // gtk
// eox

#endif //STEREOX_GTK_CONTROL_H
