//
// Created by henryco on 8/19/23.
//

#pragma once

#include <exception>

class ErrorReporter {
public:
    static void printStackTrace(const std::exception& e);

    static void printStackTrace();

    static void dumpError(const std::exception& e);

    static void dumpError();
};


