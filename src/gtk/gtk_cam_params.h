//
// Created by henryco on 11/25/23.
//

#ifndef STEREOX_GTK_CAM_PARAMS_H
#define STEREOX_GTK_CAM_PARAMS_H

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>

namespace sex::xgtk {

    class GtkCamParams;

    class GtkCamProp {
    protected:
        friend GtkCamParams;

        uint id;
        uint type;
        std::string name;
        int min;
        int max;
        int step;
        int default_value;
        int value;
    public:
        GtkCamProp(uint id, uint type, std::string name, int min,
                   int max, int step, int defaultValue, int value);

        GtkCamProp(const GtkCamProp &other) = default;

        GtkCamProp(GtkCamProp &&other) noexcept;
    };

    class GtkCamParams : public Gtk::ScrolledWindow {
    private:
        static inline const auto log =
                spdlog::stdout_color_mt("gtk_cam_params");

        // preventing memory leak for dynamically allocated widgets
        std::vector<std::unique_ptr<Gtk::Widget>> controls;

        std::function<int(uint, int)> onUpdateCallback{};
        sigc::connection debounce_connection;
        bool programmatic_change = false;

    public:
        static inline const uint DELAY_MS = 300;

        GtkCamParams();

        ~GtkCamParams() override = default;

        void onUpdate(std::function<int(uint, int)> callback);

        void setProperties(const std::vector<GtkCamProp>& properties);
    };

} // sex

#endif //STEREOX_GTK_CAM_PARAMS_H
