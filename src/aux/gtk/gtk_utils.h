//
// Created by henryco on 11/26/23.
//

#ifndef STEREOX_GTK_UTILS_H
#define STEREOX_GTK_UTILS_H

#include <gtkmm/widget.h>
#include <gtkmm/cssprovider.h>

namespace eox::xgtk {

    inline void add_style(Gtk::Widget &widget, const std::basic_string<char> &style,
                   guint priority = GTK_STYLE_PROVIDER_PRIORITY_USER) {
        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_data(style);
        widget.get_style_context()->add_provider(css_provider, priority);
    }

} // eox

#endif //STEREOX_GTK_UTILS_H
