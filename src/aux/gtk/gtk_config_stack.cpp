//
// Created by henryco on 11/28/23.
//

#include "gtk_config_stack.h"
#include "gtk_utils.h"

namespace eox::xgtk {

    GtkConfigStack::GtkConfigStack() {
        set_orientation(Gtk::ORIENTATION_VERTICAL);
        switcher.set_stack(stack);

        pack_start(switcher, Gtk::PACK_SHRINK);
        pack_start(stack);

        get_style_context()->add_class("config-stack-box");
        eox::xgtk::add_style(*this, R"css(
            .config-stack-box {
                 background-color: white;
             }
        )css");
    }

    void GtkConfigStack::add(Gtk::Widget &child, const Glib::ustring &name, const Glib::ustring &title) {
        stack.add(child, name, title);
    }

    void GtkConfigStack::add(Gtk::Widget &child, const Glib::ustring &title) {
        add(child, title, title);
    }

    void GtkConfigStack::onChange(std::function<void(std::string)> callback) {
        stack.property_visible_child_name()
                .signal_changed()
                .connect([this, c = std::move(callback)]() {
                    c(stack.get_visible_child_name());
                });
    }

} // eox::xgtk