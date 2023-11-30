//
// Created by henryco on 11/30/23.
//

#ifndef STEREOX_EVENTS_H
#define STEREOX_EVENTS_H

#include <vector>
#include <spdlog/logger.h>

namespace sex::events {

    void gtk_save_camera_settings_event(
            const std::vector<uint> &devices,
            Gtk::Window &window,
            const std::shared_ptr<spdlog::logger> log);

} // events

#endif //STEREOX_EVENTS_H
