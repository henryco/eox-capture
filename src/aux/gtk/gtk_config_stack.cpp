//
// Created by henryco on 11/28/23.
//

#include "gtk_config_stack.h"

namespace sex::xgtk {

    GtkConfigStack::GtkConfigStack() {
        set_orientation(Gtk::ORIENTATION_VERTICAL);
        switcher.set_stack(stack);

        pack_start(switcher, Gtk::PACK_SHRINK);
        pack_start(stack);
    }

    void GtkConfigStack::add(Gtk::Widget &child, const Glib::ustring &name, const Glib::ustring &title) {
        stack.add(child, name, title);
    }

    void GtkConfigStack::add(Gtk::Widget &child, const Glib::ustring &title) {
        add(child, title, title);
    }

} // sex::xgtk