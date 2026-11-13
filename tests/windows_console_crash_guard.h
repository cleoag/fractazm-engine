#pragma once

#include <cstdio>
#include <cstdlib>
#include <exception>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <crtdbg.h>

static LONG WINAPI test_console_exception_filter(EXCEPTION_POINTERS* info) {
    unsigned long code = info && info->ExceptionRecord ? info->ExceptionRecord->ExceptionCode : 0;
    std::fprintf(stderr, "Fatal Windows exception: 0x%08lX\n", code);
    std::fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void test_terminate_to_stderr() {
    std::fprintf(stderr, "Fatal error: unhandled C++ exception or terminate()\n");
    std::fflush(stderr);
    std::_Exit(3);
}

static inline void install_test_console_crash_handlers() {
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    SetUnhandledExceptionFilter(test_console_exception_filter);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    std::set_terminate(test_terminate_to_stderr);
}
#else
static inline void install_test_console_crash_handlers() {}
#endif
