// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_DEBUG_H
#define EXS_DEBUG_H

#include "../Compiler/BuildConfig.h"
#include <cstddef>

namespace Exs_ {
namespace Debug {

class ExsDebug {
public:
    struct ExsStackFrame {
        void* address;
        const char* function;
        const char* file;
        int line;
    };
    
    static void ExsInit();
    static void ExsShutdown();
    
    static void ExsBreak();
    static void ExsPrintStackTrace(int maxFrames = 64);
    
    static int ExsCaptureStackTrace(ExsStackFrame* frames, int maxFrames);
    static void ExsPrintStackTrace(const ExsStackFrame* frames, int count);
    
    static void ExsSetCrashHandler(void (*handler)(int signal));
    static void ExsRestoreDefaultCrashHandler();
    
    static void ExsEnableMemoryLeakDetection();
    static void ExsDisableMemoryLeakDetection();
    static size_t ExsGetMemoryLeakCount();
    
    static void ExsDumpMemoryLeaks();
    
    static void ExsEnableBreakOnAllocation(size_t allocationNumber);
    static void ExsDisableBreakOnAllocation();
    
    static void ExsSetAllocationHook(
        void (*allocHook)(size_t size, void* ptr),
        void (*freeHook)(void* ptr)
    );
    
    static void ExsClearAllocationHooks();
    
    static bool ExsIsDebuggerPresent();
    static void ExsOutputDebugString(const char* format, ...);
    
    static void ExsFlushDebugOutput();
    
private:
    ExsDebug() = delete;
};

} // namespace Debug
} // namespace Exs_

#endif // EXS_DEBUG_H
