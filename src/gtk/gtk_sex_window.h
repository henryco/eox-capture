//
// Created by henryco on 11/28/23.
//

#ifndef STEREOX_GTK_SEX_WINDOW_H
#define STEREOX_GTK_SEX_WINDOW_H

#include <gtkmm/window.h>
#include <glibmm/dispatcher.h>
#include "../data/data_structures.h"

namespace sex::xgtk {

    class GtkSexWindow : public Gtk::Window {

    private:
        std::vector<std::unique_ptr<Gtk::Widget>> u_widgets;
        std::vector<std::shared_ptr<Gtk::Widget>> s_widgets;
        Glib::Dispatcher dispatcher;

        void on_dispatcher_signal() {
            onRefresh();
        }

    public:
        virtual void init(const std::map<std::string, sex::data::config_value>& configuration) = 0;

        void init() {
            init(std::map<std::string, sex::data::config_value>());
        }

        virtual ~GtkSexWindow() = default; // NOLINT(*-use-override)

        GtkSexWindow() {
            dispatcher.connect(sigc::mem_fun(*this, &GtkSexWindow::on_dispatcher_signal));
        }

    protected:

        virtual void onRefresh() { /* Nothing goings-on here */ }

        void refresh() {
            dispatcher.emit();
        }

        void keep(std::unique_ptr<Gtk::Widget> &&widget) {
            u_widgets.push_back(std::move(widget));
        }

        void keep_shared(std::shared_ptr<Gtk::Widget> &widget) {
            s_widgets.push_back(widget);
        }

        void keep_shared(std::shared_ptr<Gtk::Widget> &&widget) {
            s_widgets.push_back(widget);
        }

        void un_keep() {
            u_widgets.clear();
            s_widgets.clear();
        }
    };

} // sex

#endif //STEREOX_GTK_SEX_WINDOW_H
