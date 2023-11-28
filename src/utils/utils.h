//
// Created by henryco on 11/28/23.
//

#ifndef STEREOX_UTILS_H
#define STEREOX_UTILS_H

#include <string>

namespace sex::utils {

    std::string to_string(const u_char *str_array, size_t length) {
        return {reinterpret_cast<const char *>(str_array), length};
    }

}

#endif //STEREOX_UTILS_H
