//
// Created by henryco on 11/25/23.
//

#include <gtkmm/cssprovider.h>
#include "gtk_cam_params.h"

namespace sex::xgtk {

    GtkCamProp::GtkCamProp(
            uint id,
            uint type,
            std::string name,
            int min,
            int max,
            int step,
            int defaultValue,
            int value) :
            id(id),
            type(type),
            name(std::move(name)),
            min(min),
            max(max),
            step(step),
            default_value(defaultValue),
            value(value) {};

    GtkCamProp::GtkCamProp(GtkCamProp &&other) noexcept
            : id(other.id),
              type(other.type),
              name(std::move(other.name)),
              min(other.min),
              max(other.max),
              step(other.step),
              default_value(other.default_value),
              value(other.value) {}

    GtkCamParams::GtkCamParams() {
        set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        pack_start(v_box, Gtk::PACK_SHRINK);
    }

    void GtkCamParams::onUpdate(std::function<int(uint, int)> _callback) {
        this->onUpdateCallback = std::move(_callback);
    }

    void GtkCamParams::setProperties(std::vector<GtkCamProp> _properties) {
        for (auto &control: controls) {
            v_box.remove(*control);
        }
        properties.clear();
        controls.clear();

        properties.reserve(_properties.size());
        controls.reserve(_properties.size());

        for (auto &property: _properties) {
            properties.push_back(std::move(property));

            auto box = std::make_unique<Gtk::Box>();
            {
                // some extra elements and props here
                box->get_style_context()->add_class("cam-param-box");
                box->set_size_request(300, 100);

                auto css_provider = Gtk::CssProvider::create();
                css_provider->load_from_data(".cam-param-box { background-color: blue; color: white; }");
                box->get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
            }

            v_box.pack_start(*box, Gtk::PACK_SHRINK);
            controls.push_back(std::move(box));
        }

    }

} // sex
