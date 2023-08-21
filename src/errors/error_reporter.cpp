//
// Created by henryco on 8/19/23.
//

#include <iostream>
#include "error_reporter.h"

void ErrorReporter::dumpError(const std::exception &e) {
    // TODO
}

void ErrorReporter::printStackTrace(const std::exception &e) {
    std::cerr << "Debug Report: An exception occurred: " << e.what() << '\n';
}

#ifdef _WIN32 // Windows platform
#include <windows.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

void ErrorReporter::void printStackTrace() {
    // Honestly I have no idea if this would work

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    void* stack[100];
    unsigned short frames = CaptureStackBackTrace(0, 100, stack, NULL);

    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (unsigned int i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        std::cerr << frames - i - 1 << ": " << symbol->Name << " at 0x" << std::hex << symbol->Address << std::dec << std::endl;
    }

    free(symbol);
}

void ErrorReporter::dumpError() {
    // TODO
}

#else // UNIX platform
#include <execinfo.h>

void ErrorReporter::printStackTrace() {
    void* array[10];
    int size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames
    char** messages = backtrace_symbols(array, size);

    for (size_t i = 0; i < size; ++i) { // Using size_t for loop counter
        std::cerr << messages[i] << std::endl;
    }

    free(messages);
}

void ErrorReporter::dumpError() {
    // TODO
}

#endif