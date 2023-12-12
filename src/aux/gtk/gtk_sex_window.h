//
// Created by henryco on 11/28/23.
//

#ifndef STEREOX_GTK_SEX_WINDOW_H
#define STEREOX_GTK_SEX_WINDOW_H

#include <gtkmm/window.h>
#include <glibmm/dispatcher.h>
#include "../commons.h"

namespace sex::xgtk {

    class GtkSexWindow : public Gtk::Window {

    private:
        std::vector<std::unique_ptr<Gtk::Widget>> u_widgets;
        std::vector<std::shared_ptr<Gtk::Widget>> s_widgets;
        Glib::Dispatcher dispatcher;

        void on_dispatcher_signal();

    public:
        virtual void init(sex::data::basic_config configuration) = 0;

        void init();

        virtual ~GtkSexWindow() = default; // NOLINT(*-use-override)

        GtkSexWindow();

    protected:

        virtual void onResize(const Gtk::Allocation& allocation);

        virtual void onRefresh();

        void refresh();

        void keep(std::unique_ptr<Gtk::Widget> &&widget);

        void keep_shared(std::shared_ptr<Gtk::Widget> &widget);

        void keep_shared(std::shared_ptr<Gtk::Widget> &&widget);

        void un_keep();
    };

} // sex

#endif //STEREOX_GTK_SEX_WINDOW_H
