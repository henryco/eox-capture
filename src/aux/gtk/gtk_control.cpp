//
// Created by henryco on 12/9/23.
//

#include "gtk_control.h"
#include "gtk_utils.h"

#include <utility>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>
#include <glibmm/main.h>

namespace eox::gtk {

    GtkControl::GtkControl(
            std::function<double(double value)> callback,
            std::string label,
            double value,
            double step,
            double def_value,
            double min_value,
            double max_value)
            : GtkControl(std::move(label), value, step, def_value, min_value, max_value) {
        setCallback(std::move(callback));
    }

    GtkControl::GtkControl(std::string label, double value, double step, double def_value,
                           double min_value, double max_value)
            : label(std::move(label)), value(value), step(step),
              def_value(def_value), min_value(min_value), max_value(max_value) {
        set_orientation(Gtk::ORIENTATION_VERTICAL);
    }

    GtkControl &GtkControl::setCallback(std::function<double(double)> _callback) {
        this->callback = std::move(_callback);

        for (const auto &widget: controls) {
            remove(*widget);
        }
        controls.clear();

        auto h_box = std::make_unique<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        auto scale = std::make_unique<Gtk::Scale>(Gtk::ORIENTATION_HORIZONTAL);
        auto entry = std::make_unique<Gtk::SpinButton>();
        auto text = std::make_unique<Gtk::Label>();

        {
            // Control container
            set_size_request(300, 100);
            get_style_context()->add_class("cam-param-c_box");
            sex::xgtk::add_style(*this, R"css(
                    .cam-param-c_box {
                        background-color: white;
                        border-bottom: 1px solid lightgrey;
                        padding-right: 20px;
                        padding-left: 15px;
                        padding-top: 10px;
                    }
                )css");
        }

        {
            // Label
            text->set_text(label);
            text->set_alignment(0);
            text->get_style_context()->add_class("cam-param-label");
            sex::xgtk::add_style(*text, R"css(
                        .cam-param-label {
                            padding-bottom: 10px;
                        }
                    )css");
            pack_start(*text, Gtk::PACK_SHRINK);
        }

        {
            // Slider
            scale->set_size_request(260);
            scale->set_range(min_value, max_value);
            scale->set_increments(step, step);
            scale->set_digits(0);
            scale->set_value_pos(Gtk::POS_TOP);
            scale->set_value(value);

            scale->signal_value_changed().connect([s_ptr = &*scale, e_ptr = &*entry, this]() {
                if (programmatic_change)
                    return;
                programmatic_change = true;
                {
                    auto _value = s_ptr->get_value();
                    const auto reminder = std::fmod(_value, step);
                    if (reminder != 0) {
                        _value = _value + step - reminder;
                        s_ptr->set_value(_value);
                    }
                    e_ptr->set_value(_value);

                    if (debounce_connection.connected())
                        debounce_connection.disconnect();

                    debounce_connection = Glib::signal_timeout().connect([this, _value, e_ptr, s_ptr]() {
                        programmatic_change = true;
                        {
                            const auto result = callback(_value);
                            if (_value != result)
                                s_ptr->set_value(result);
                            e_ptr->set_value(result);
                        }
                        programmatic_change = false;
                        return false;
                    }, DELAY_MS);
                }
                programmatic_change = false;
            });

            h_box->pack_start(*scale, Gtk::PACK_SHRINK);
        }

        {
            // Spin button
            entry->set_numeric(true);
            entry->set_snap_to_ticks(true);
            entry->set_update_policy(Gtk::UPDATE_IF_VALID);
            entry->set_size_request(50, 50);
            entry->set_range(min_value, max_value);
            entry->set_increments(step, step);
            entry->set_digits(0);
            entry->set_width_chars(5);
            entry->set_value(value);
            entry->signal_value_changed().connect([s_ptr = &*scale, e_ptr = &*entry, this]() {
                if (programmatic_change)
                    return;
                programmatic_change = true;
                {
                    auto _value = e_ptr->get_value();
                    const auto reminder = std::fmod(_value, step);
                    if (reminder != 0) {
                        _value = _value + step - reminder;
                        e_ptr->set_value(_value);
                    }
                    s_ptr->set_value(_value);

                    if (debounce_connection.connected())
                        debounce_connection.disconnect();

                    debounce_connection = Glib::signal_timeout().connect([this, _value, e_ptr, s_ptr]() {
                        programmatic_change = true;
                        {
                            const auto result = callback(_value);
                            if (_value != result)
                                e_ptr->set_value(result);
                            s_ptr->set_value(result);
                        }
                        programmatic_change = false;
                        return false;
                    }, DELAY_MS);
                }
                programmatic_change = false;
            });

            h_box->pack_end(*entry, Gtk::PACK_SHRINK);
        }

        pack_start(*h_box, Gtk::PACK_SHRINK);

        controls.push_back(std::move(h_box));
        controls.push_back(std::move(scale));
        controls.push_back(std::move(entry));
        controls.push_back(std::move(text));
        return *this;
    }

    void GtkControl::reset() {
        value = def_value;
        setCallback(callback);
    }

} // eox