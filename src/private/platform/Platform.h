#pragma once
#ifndef EXS_PLATFORM_H
#define EXS_PLATFORM_H

#include "PlatformDetection.h"
#include "SystemInfo.h"
#include "CPUInfo.h"
#include "MemoryInfo.h"
#include "Endian.h"
#include "PlatformUtils.h"
#include "../Types/BasicTypes.h"
#include "../Debug/Assert.h"
#include <cstddef>

namespace Exs {
namespace Internal {
namespace Platform {

// Platform abstraction layer
class EXS_API Exs_Platform {
private:
    static bool Exs_initialized;
    static Exs_uint64 Exs_performanceCounterFrequency;
    static double Exs_secondsPerCycle;
    
    // Singleton pattern
    Exs_Platform() = delete;
    ~Exs_Platform() = delete;
    Exs_Platform(const Exs_Platform&) = delete;
    Exs_Platform& operator=(const Exs_Platform&) = delete;
    
public:
    // Initialization and shutdown
    static void Exs_Initialize();
    static void Exs_Shutdown();
    static bool Exs_IsInitialized() { return Exs_initialized; }
    
    // Time functions
    static Exs_uint64 Exs_GetPerformanceCounter();
    static Exs_uint64 Exs_GetPerformanceFrequency();
    static double Exs_GetTime();
    static double Exs_GetHighPrecisionTime();
    
    // Sleep and yield
    static void Exs_Sleep(Exs_uint32 milliseconds);
    static void Exs_SleepMicroseconds(Exs_uint32 microseconds);
    static void Exs_SleepNanoseconds(Exs_uint64 nanoseconds);
    static void Exs_YieldThread();
    static void Exs_YieldProcessor();
    
    // Memory allocation
    static void* Exs_AllocateAligned(Exs_size_t size, Exs_size_t alignment);
    static void Exs_FreeAligned(void* ptr);
    static void* Exs_ReallocateAligned(void* ptr, Exs_size_t newSize, Exs_size_t alignment);
    
    // Memory pages
    static void* Exs_AllocatePage(Exs_size_t size);
    static void Exs_FreePage(void* ptr, Exs_size_t size);
    static bool Exs_ProtectPage(void* ptr, Exs_size_t size, bool read, bool write, bool execute);
    static Exs_size_t Exs_GetPageSize();
    static Exs_size_t Exs_GetAllocationGranularity();
    
    // Memory barriers
    static void Exs_MemoryBarrier();
    static void Exs_ReadBarrier();
    static void Exs_WriteBarrier();
    static void Exs_CompilerBarrier();
    
    // Atomic operations
    static Exs_int32 Exs_AtomicIncrement(volatile Exs_int32* value);
    static Exs_int64 Exs_AtomicIncrement64(volatile Exs_int64* value);
    static Exs_int32 Exs_AtomicDecrement(volatile Exs_int32* value);
    static Exs_int64 Exs_AtomicDecrement64(volatile Exs_int64* value);
    static Exs_int32 Exs_AtomicAdd(volatile Exs_int32* value, Exs_int32 add);
    static Exs_int64 Exs_AtomicAdd64(volatile Exs_int64* value, Exs_int64 add);
    static Exs_int32 Exs_AtomicExchange(volatile Exs_int32* value, Exs_int32 exchange);
    static Exs_int64 Exs_AtomicExchange64(volatile Exs_int64* value, Exs_int64 exchange);
    static Exs_int32 Exs_AtomicCompareExchange(volatile Exs_int32* dest, Exs_int32 exchange, Exs_int32 comparand);
    static Exs_int64 Exs_AtomicCompareExchange64(volatile Exs_int64* dest, Exs_int64 exchange, Exs_int64 comparand);
    
    // Thread local storage
    static Exs_uint32 Exs_TLSAlloc();
    static bool Exs_TLSFree(Exs_uint32 index);
    static void* Exs_TLSGetValue(Exs_uint32 index);
    static bool Exs_TLSSetValue(Exs_uint32 index, void* value);
    
    // Thread functions
    static Exs_uint32 Exs_GetCurrentThreadId();
    static void Exs_SetThreadName(const char* name);
    static void Exs_SetThreadPriority(Exs_int32 priority);
    static void Exs_SetThreadAffinityMask(Exs_uint64 mask);
    
    // Process functions
    static Exs_uint32 Exs_GetCurrentProcessId();
    static Exs_size_t Exs_GetProcessMemoryUsage();
    static Exs_size_t Exs_GetProcessPeakMemoryUsage();
    static void Exs_SetProcessPriority(Exs_int32 priority);
    
    // Module handling
    static void* Exs_LoadLibrary(const char* filename);
    static bool Exs_FreeLibrary(void* handle);
    static void* Exs_GetProcAddress(void* handle, const char* procname);
    
    // File system
    static bool Exs_CreateDirectory(const char* path);
    static bool Exs_DeleteDirectory(const char* path);
    static bool Exs_CreateFile(const char* filename);
    static bool Exs_DeleteFile(const char* filename);
    static bool Exs_FileExists(const char* filename);
    static bool Exs_DirectoryExists(const char* path);
    static Exs_uint64 Exs_GetFileSize(const char* filename);
    static Exs_uint64 Exs_GetFileLastModifiedTime(const char* filename);
    static bool Exs_RenameFile(const char* oldName, const char* newName);
    static bool Exs_CopyFile(const char* source, const char* destination);
    
    // Path utilities
    static char Exs_GetPathSeparator();
    static void Exs_NormalizePath(char* path);
    static void Exs_GetAbsolutePath(const char* relativePath, char* absolutePath, Exs_size_t bufferSize);
    static void Exs_GetCurrentDirectory(char* buffer, Exs_size_t bufferSize);
    static bool Exs_SetCurrentDirectory(const char* path);
    static void Exs_GetExecutablePath(char* buffer, Exs_size_t bufferSize);
    static void Exs_GetModulePath(void* module, char* buffer, Exs_size_t bufferSize);
    static void Exs_GetTempPath(char* buffer, Exs_size_t bufferSize);
    static void Exs_GetHomeDirectory(char* buffer, Exs_size_t bufferSize);
    
    // Environment variables
    static bool Exs_GetEnvironmentVariable(const char* name, char* buffer, Exs_size_t bufferSize);
    static bool Exs_SetEnvironmentVariable(const char* name, const char* value);
    
    // Console
    static bool Exs_AttachConsole();
    static bool Exs_FreeConsole();
    static bool Exs_AllocConsole();
    static void Exs_WriteConsole(const char* text);
    static void Exs_WriteConsoleError(const char* text);
    
    // Debugging
    static void Exs_DebugBreak();
    static void Exs_OutputDebugString(const char* message);
    static bool Exs_IsDebuggerPresent();
    static void Exs_BreakIntoDebugger();
    
    // Error handling
    static Exs_uint32 Exs_GetLastError();
    static void Exs_SetLastError(Exs_uint32 error);
    static const char* Exs_GetErrorString(Exs_uint32 error);
    
    // System information
    static const Exs_SystemInfo& Exs_GetSystemInfo();
    static const Exs_CPUInfo& Exs_GetCPUInfo();
    static const Exs_MemoryInfo& Exs_GetMemoryInfo();
    
    // Platform-specific features
    static bool Exs_IsElevated();
    static bool Exs_IsService();
    static bool Exs_IsUserAdmin();
    
    // Power management
    static bool Exs_IsOnBatteryPower();
    static Exs_uint32 Exs_GetBatteryLifePercent();
    static bool Exs_GetPowerStatus(bool* acOnline, bool* batteryCharging, Exs_uint32* batteryLifePercent, Exs_uint32* batteryLifeTime);
    
    // Clipboard
    static bool Exs_GetClipboardText(char* buffer, Exs_size_t bufferSize);
    static bool Exs_SetClipboardText(const char* text);
    
    // UUID generation
    static void Exs_GenerateUUID(char* buffer, Exs_size_t bufferSize);
    
    // Random number generation (cryptographically secure if available)
    static bool Exs_GenerateRandomBytes(void* buffer, Exs_size_t size);
    
    // Command line
    static Exs_int32 Exs_GetCommandLineArgCount();
    static const char* Exs_GetCommandLineArg(Exs_int32 index);
    static const char* Exs_GetCommandLine();
    
    // Application exit
    static void Exs_ExitProcess(Exs_int32 exitCode);
    static void Exs_AbortProcess();
    static void Exs_TerminateProcess(Exs_uint32 processId, Exs_int32 exitCode);
    
    // Dynamic code generation
    static void* Exs_AllocateExecutableMemory(Exs_size_t size);
    static void Exs_FreeExecutableMemory(void* ptr, Exs_size_t size);
    static bool Exs_FlushInstructionCache(void* ptr, Exs_size_t size);
    
    // Signal/exception handling
    static bool Exs_SetSignalHandler(Exs_int32 signal, void (*handler)(Exs_int32));
    static bool Exs_SetUnhandledExceptionFilter(void (*filter)(void*));
    
    // Critical sections
    static void* Exs_CreateCriticalSection();
    static void Exs_DeleteCriticalSection(void* cs);
    static void Exs_EnterCriticalSection(void* cs);
    static bool Exs_TryEnterCriticalSection(void* cs);
    static void Exs_LeaveCriticalSection(void* cs);
    
    // Event objects
    static void* Exs_CreateEvent(bool manualReset, bool initialState, const char* name = nullptr);
    static void Exs_DestroyEvent(void* event);
    static bool Exs_SetEvent(void* event);
    static bool Exs_ResetEvent(void* event);
    static bool Exs_PulseEvent(void* event);
    static Exs_uint32 Exs_WaitForSingleObject(void* handle, Exs_uint32 milliseconds);
    static Exs_uint32 Exs_WaitForMultipleObjects(Exs_uint32 count, void** handles, bool waitAll, Exs_uint32 milliseconds);
    
    // Mutex objects
    static void* Exs_CreateMutex(bool initialOwner, const char* name = nullptr);
    static void Exs_DestroyMutex(void* mutex);
    static bool Exs_ReleaseMutex(void* mutex);
    
    // Semaphore objects
    static void* Exs_CreateSemaphore(Exs_int32 initialCount, Exs_int32 maximumCount, const char* name = nullptr);
    static void Exs_DestroySemaphore(void* semaphore);
    static bool Exs_ReleaseSemaphore(void* semaphore, Exs_int32 releaseCount, Exs_int32* previousCount);
    
    // Timer functions
    static void* Exs_CreateTimer(bool manualReset, void (*callback)(void*), void* userData);
    static void Exs_DestroyTimer(void* timer);
    static bool Exs_SetTimer(void* timer, Exs_uint32 period, bool periodic);
    static bool Exs_CancelTimer(void* timer);
    
    // Window message processing (Windows specific)
#ifdef EXS_PLATFORM_WINDOWS
    static bool Exs_PumpMessage();
    static bool Exs_PeekMessage(void* msg);
    static bool Exs_TranslateMessage(void* msg);
    static bool Exs_DispatchMessage(void* msg);
    static void Exs_PostQuitMessage(Exs_int32 exitCode);
#endif
};

// Inline helper functions
inline void Exs_PlatformSleep(Exs_uint32 milliseconds) {
    Exs_Platform::Exs_Sleep(milliseconds);
}

inline double Exs_PlatformGetTime() {
    return Exs_Platform::Exs_GetTime();
}

inline void* Exs_PlatformAllocateAligned(Exs_size_t size, Exs_size_t alignment = 16) {
    return Exs_Platform::Exs_AllocateAligned(size, alignment);
}

inline void Exs_PlatformFreeAligned(void* ptr) {
    Exs_Platform::Exs_FreeAligned(ptr);
}

inline void Exs_PlatformMemoryBarrier() {
    Exs_Platform::Exs_MemoryBarrier();
}

inline void Exs_PlatformDebugBreak() {
    Exs_Platform::Exs_DebugBreak();
}

} // namespace Platform
} // namespace Internal
} // namespace Exs

#endif // EXS_PLATFORM_H
