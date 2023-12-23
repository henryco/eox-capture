#include "gtk_eox_window.h"

namespace eox::xgtk {

    GtkEoxWindow::GtkEoxWindow() {
        dispatcher.connect(sigc::mem_fun(*this, &GtkEoxWindow::on_dispatcher_signal));
        signal_size_allocate().connect([this](const Gtk::Allocation& allocation) {
            onResize(allocation);
        });
    }

    void GtkEoxWindow::refresh() {
        dispatcher.emit();
    }

    void GtkEoxWindow::keep(std::unique_ptr<Gtk::Widget> &&widget) {
        u_widgets.push_back(std::move(widget));
    }

    void GtkEoxWindow::keep_shared(std::shared_ptr<Gtk::Widget> &widget) {
        s_widgets.push_back(widget);
    }

    void GtkEoxWindow::keep_shared(std::shared_ptr<Gtk::Widget> &&widget) {
        s_widgets.push_back(widget);
    }

    void GtkEoxWindow::un_keep() {
        u_widgets.clear();
        s_widgets.clear();
    }

    void GtkEoxWindow::onResize(const Gtk::Allocation &allocation) {
        /* Nothing goings-on here */
    }

    void GtkEoxWindow::onRefresh() {
        /* Nothing goings-on here */
    }

    void GtkEoxWindow::init() {
        init({});
    }

    void GtkEoxWindow::on_dispatcher_signal() {
        onRefresh();
    }
}