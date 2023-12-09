//
// Created by henryco on 12/9/23.
//

#ifndef STEREOX_GTK_CONTROL_H
#define STEREOX_GTK_CONTROL_H


#include <gtkmm/box.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace eox::gtk {

    class GtkControl : public Gtk::Box {
        static inline const auto log =
                spdlog::stdout_color_mt("gtk_control");

    private:
        // preventing memory leak for dynamically allocated u_widgets
        std::vector<std::unique_ptr<Gtk::Widget>> controls;

        std::function<double(std::string name, double value)> callback;
        sigc::connection debounce_connection;

        std::string id, label;
        bool programmatic_change = false;
        double def_value, min_value, max_value, value, step;

    public:
        static inline const uint DELAY_MS = 500;

        GtkControl(
                std::string id,
                std::string label,
                double value,
                double step = 1,
                double def_value = 0,
                double min_value = 0,
                double max_value = 8192);

        ~GtkControl() override = default;

        GtkControl& setCallback(std::function<double(std::string name, double value)> callback);

        void reset();
    };

} // gtk
// eox

#endif //STEREOX_GTK_CONTROL_H
