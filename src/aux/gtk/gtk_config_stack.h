//
// Created by henryco on 11/28/23.
//

#ifndef STEREOX_GTK_CONFIG_STACK_H
#define STEREOX_GTK_CONFIG_STACK_H

#include <gtkmm/box.h>
#include <gtkmm/stack.h>
#include <gtkmm/stackswitcher.h>

namespace eox::xgtk {

    class GtkConfigStack : public Gtk::Box {
    private:
        Gtk::Stack stack;
        Gtk::StackSwitcher switcher;

    public:
        GtkConfigStack();

        void add(Widget &child, const Glib::ustring &name, const Glib::ustring &title);

        void add(Widget &child, const Glib::ustring &title);

        void onChange(std::function<void(std::string name)> callback);
    };

} // eox

#endif //STEREOX_GTK_CONFIG_STACK_H
