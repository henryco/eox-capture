#include "gtk_sex_window.h"

namespace sex::xgtk {

    GtkSexWindow::GtkSexWindow() {
        dispatcher.connect(sigc::mem_fun(*this, &GtkSexWindow::on_dispatcher_signal));
        signal_size_allocate().connect([this](const Gtk::Allocation& allocation) {
            onResize(allocation);
        });
    }

    void GtkSexWindow::refresh() {
        dispatcher.emit();
    }

    void GtkSexWindow::keep(std::unique_ptr<Gtk::Widget> &&widget) {
        u_widgets.push_back(std::move(widget));
    }

    void GtkSexWindow::keep_shared(std::shared_ptr<Gtk::Widget> &widget) {
        s_widgets.push_back(widget);
    }

    void GtkSexWindow::keep_shared(std::shared_ptr<Gtk::Widget> &&widget) {
        s_widgets.push_back(widget);
    }

    void GtkSexWindow::un_keep() {
        u_widgets.clear();
        s_widgets.clear();
    }

    void GtkSexWindow::onResize(const Gtk::Allocation &allocation) {
        /* Nothing goings-on here */
    }

    void GtkSexWindow::onRefresh() {
        /* Nothing goings-on here */
    }

    void GtkSexWindow::init() {
        init({});
    }

    void GtkSexWindow::on_dispatcher_signal() {
        onRefresh();
    }
}