//
// Created by henryco on 11/28/23.
//

#ifndef STEREOX_GTK_SEX_WINDOW_H
#define STEREOX_GTK_SEX_WINDOW_H

#include <gtkmm/window.h>
#include <glibmm/dispatcher.h>

namespace sex {

    class GtkSexWindow : public Gtk::Window {
    public:
        virtual void init() = 0;

        GtkSexWindow() {
            dispatcher.connect(sigc::mem_fun(*this, &GtkSexWindow::on_dispatcher_signal));
        }

        virtual ~GtkSexWindow() = default; // NOLINT(*-use-override)

    protected:

        virtual void onRefresh() { /* Nothing goings-on here */ }

        void emit() {
            dispatcher.emit();
        }

        void keep(std::unique_ptr<Gtk::Widget> &widget) {
            u_widgets.push_back(std::move(widget));
        }

        void keep(std::unique_ptr<Gtk::Widget> &&widget) {
            u_widgets.push_back(std::move(widget));
        }

        void keep(std::shared_ptr<Gtk::Widget> &widget) {
            s_widgets.push_back(widget);
        }

        void keep(std::shared_ptr<Gtk::Widget> &&widget) {
            s_widgets.push_back(widget);
        }

        void un_keep() {
            u_widgets.clear();
            s_widgets.clear();
        }

    private:
        std::vector<std::unique_ptr<Gtk::Widget>> u_widgets;
        std::vector<std::shared_ptr<Gtk::Widget>> s_widgets;
        Glib::Dispatcher dispatcher;

        void on_dispatcher_signal() {
            onRefresh();
        }
    };

} // sex

#endif //STEREOX_GTK_SEX_WINDOW_H
