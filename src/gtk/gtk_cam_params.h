//
// Created by henryco on 11/25/23.
//

#ifndef STEREOX_GTK_CAM_PARAMS_H
#define STEREOX_GTK_CAM_PARAMS_H

#include <gtkmm/box.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace sex::xgtk {

    class GtkCamProp {
    protected:
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

        GtkCamProp(const GtkCamProp& other) = default;

        GtkCamProp(GtkCamProp&& other) noexcept;
    };

    class GtkCamParams : public Gtk::Box {
    private:
        static inline const auto log =
                spdlog::stdout_color_mt("gtk_cam_params");

        Gtk::Box v_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);

        std::vector<std::unique_ptr<Gtk::Box>> controls;
        std::vector<GtkCamProp> properties;

        std::function<int(uint, int)> onUpdateCallback;

    public:
        GtkCamParams();

        ~GtkCamParams() override = default;

        void onUpdate(std::function<int(uint, int)> callback);

        void setProperties(std::vector<GtkCamProp> properties);
    };

} // sex

#endif //STEREOX_GTK_CAM_PARAMS_H
