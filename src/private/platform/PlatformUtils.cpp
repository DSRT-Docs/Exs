// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#include "PlatformUtils.h"
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <psapi.h>
    #include <lmcons.h>
    #include <powrprof.h>
    #pragma comment(lib, "powrprof.lib")
#elif defined(__linux__)
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <dirent.h>
    #include <cstdlib>
    #include <fstream>
#elif defined(__APPLE__)
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/sysctl.h>
    #include <mach/mach_time.h>
#endif

namespace Exs {
namespace Platform {

std::string Exs_PlatformUtils::Exs_GetCurrentDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    return buffer;
#else
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return buffer;
    }
    return "";
#endif
}

bool Exs_PlatformUtils::Exs_SetCurrentDirectory(const std::string& path) {
#ifdef _WIN32
    return SetCurrentDirectoryA(path.c_str()) != FALSE;
#else
    return chdir(path.c_str()) == 0;
#endif
}

std::string Exs_PlatformUtils::Exs_GetHomeDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, buffer) == S_OK) {
        return buffer;
    }
    return "";
#else
    const char* home = getenv("HOME");
    if (home != nullptr) {
        return home;
    }
    
    // Fallback to password database
    struct passwd* pw = getpwuid(getuid());
    if (pw != nullptr) {
        return pw->pw_dir;
    }
    
    return "";
#endif
}

std::string Exs_PlatformUtils::Exs_GetTempDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetTempPathA(MAX_PATH, buffer);
    return buffer;
#else
    const char* tmpdir = getenv("TMPDIR");
    if (tmpdir != nullptr) {
        return tmpdir;
    }
    return "/tmp";
#endif
}

std::string Exs_PlatformUtils::Exs_GetAppDataDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buffer) == S_OK) {
        return buffer;
    }
    return "";
#else
    std::string home = Exs_GetHomeDirectory();
    if (!home.empty()) {
        return home + "/.config";
    }
    return "";
#endif
}

std::string Exs_PlatformUtils::Exs_GetExecutablePath() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return buffer;
#elif defined(__linux__)
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return buffer;
    }
    return "";
#elif defined(__APPLE__)
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return buffer;
    }
    return "";
#endif
}

std::string Exs_PlatformUtils::Exs_GetModuleDirectory() {
    std::string exePath = Exs_GetExecutablePath();
    size_t pos = exePath.find_last_of("\\/");
    if (pos != std::string::npos) {
        return exePath.substr(0, pos);
    }
    return "";
}

uint32_t Exs_PlatformUtils::Exs_GetProcessId() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

uint32_t Exs_PlatformUtils::Exs_GetParentProcessId() {
#ifdef _WIN32
    return 0; // Would need CreateToolhelp32Snapshot
#else
    return getppid();
#endif
}

uint32_t Exs_PlatformUtils::Exs_GetThreadId() {
#ifdef _WIN32
    return GetCurrentThreadId();
#else
    return 0; // pthread_self() returns pthread_t, not integer
#endif
}

std::string Exs_PlatformUtils::Exs_GetProcessName() {
    std::string path = Exs_GetExecutablePath();
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

uint64_t Exs_PlatformUtils::Exs_GetProcessStartTime() {
#ifdef _WIN32
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, 
                       &kernelTime, &userTime)) {
        ULARGE_INTEGER largeInt;
        largeInt.LowPart = createTime.dwLowDateTime;
        largeInt.HighPart = createTime.dwHighDateTime;
        return largeInt.QuadPart;
    }
#endif
    return 0;
}

void Exs_PlatformUtils::Exs_Sleep(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void Exs_PlatformUtils::Exs_YieldProcessor() {
#ifdef _WIN32
    YieldProcessor();
#elif defined(__x86_64__) || defined(__i386__)
    __asm__ volatile("pause");
#else
    // Generic yield
    std::this_thread::yield();
#endif
}

uint64_t Exs_PlatformUtils::Exs_GetTickCount() {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
#endif
}

uint64_t Exs_PlatformUtils::Exs_GetHighResolutionCounter() {
#ifdef _WIN32
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#elif defined(__APPLE__)
    return mach_absolute_time();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (static_cast<uint64_t>(ts.tv_sec) * 1000000000) + ts.tv_nsec;
#endif
}

uint64_t Exs_PlatformUtils::Exs_GetHighResolutionFrequency() {
#ifdef _WIN32
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
#elif defined(__APPLE__)
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return (info.denom * 1000000000) / info.numer;
#else
    return 1000000000; // nanoseconds per second
#endif
}

double Exs_PlatformUtils::Exs_GetSystemTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

bool Exs_PlatformUtils::Exs_IsConsoleAttached() {
#ifdef _WIN32
    return GetConsoleWindow() != NULL;
#else
    return isatty(fileno(stdout));
#endif
}

void Exs_PlatformUtils::Exs_WriteToConsole(const std::string& text) {
    std::cout << text;
}

void Exs_PlatformUtils::Exs_WriteErrorToConsole(const std::string& text) {
    std::cerr << text;
}

void Exs_PlatformUtils::Exs_SetConsoleColor(uint32_t color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
#else
    // ANSI color codes
    std::cout << "\033[" << color << "m";
#endif
}

void Exs_PlatformUtils::Exs_ResetConsoleColor() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7); // Default white on black
#else
    std::cout << "\033[0m";
#endif
}

bool Exs_PlatformUtils::Exs_IsStdoutRedirected() {
#ifdef _WIN32
    DWORD fileType = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
    return fileType != FILE_TYPE_CHAR;
#else
    return !isatty(fileno(stdout));
#endif
}

bool Exs_PlatformUtils::Exs_IsStderrRedirected() {
#ifdef _WIN32
    DWORD fileType = GetFileType(GetStdHandle(STD_ERROR_HANDLE));
    return fileType != FILE_TYPE_CHAR;
#else
    return !isatty(fileno(stderr));
#endif
}

std::string Exs_PlatformUtils::Exs_GetEnvironmentVariable(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
}

bool Exs_PlatformUtils::Exs_SetEnvironmentVariable(const std::string& name, const std::string& value) {
#ifdef _WIN32
    return SetEnvironmentVariableA(name.c_str(), value.c_str()) != FALSE;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

bool Exs_PlatformUtils::Exs_HasEnvironmentVariable(const std::string& name) {
    return std::getenv(name.c_str()) != nullptr;
}

void Exs_PlatformUtils::Exs_DumpEnvironmentVariables() {
#ifdef _WIN32
    LPCH env = GetEnvironmentStrings();
    if (env != nullptr) {
        LPCH var = env;
        while (*var) {
            std::cout << var << std::endl;
            var += strlen(var) + 1;
        }
        FreeEnvironmentStrings(env);
    }
#else
    extern char** environ;
    for (char** env = environ; *env != nullptr; env++) {
        std::cout << *env << std::endl;
    }
#endif
}

int Exs_PlatformUtils::Exs_ExecuteCommand(const std::string& command, std::string* output) {
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    
    if (!pipe) return -1;
    
    if (output != nullptr) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output->append(buffer);
        }
    }
    
#ifdef _WIN32
    return _pclose(pipe);
#else
    return pclose(pipe);
#endif
}

int Exs_PlatformUtils::Exs_ExecuteCommandWithTimeout(const std::string& command, 
                                                   uint32_t timeout_ms,
                                                   std::string* output) {
    // Simple implementation - execute and wait
    return Exs_ExecuteCommand(command, output);
}

std::string Exs_PlatformUtils::Exs_GetComputerName() {
#ifdef _WIN32
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return buffer;
    }
    return "";
#else
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        return buffer;
    }
    return "";
#endif
}

std::string Exs_PlatformUtils::Exs_GetUserName() {
#ifdef _WIN32
    char buffer[UNLEN + 1];
    DWORD size = sizeof(buffer);
    if (GetUserNameA(buffer, &size)) {
        return buffer;
    }
    return "";
#else
    const char* user = getenv("USER");
    if (user != nullptr) {
        return user;
    }
    
    struct passwd* pw = getpwuid(getuid());
    if (pw != nullptr) {
        return pw->pw_name;
    }
    
    return "";
#endif
}

std::string Exs_PlatformUtils::Exs_GetDomainName() {
#ifdef _WIN32
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetComputerNameExA(ComputerNameDnsDomain, buffer, &size)) {
        return buffer;
    }
    return "";
#else
    char buffer[256];
    if (getdomainname(buffer, sizeof(buffer)) == 0) {
        return buffer;
    }
    return "";
#endif
}

bool Exs_PlatformUtils::Exs_IsAdministrator() {
#ifdef _WIN32
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &adminGroup)) {
        
        if (!CheckTokenMembership(nullptr, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        
        FreeSid(adminGroup);
    }
    
    return isAdmin != FALSE;
#else
    return geteuid() == 0;
#endif
}

bool Exs_PlatformUtils::Exs_IsServiceAccount() {
    std::string user = Exs_GetUserName();
    
    // Common service account names
    if (user == "SYSTEM" || user == "LOCAL SERVICE" || user == "NETWORK SERVICE") {
        return true;
    }
    
#ifdef __linux__
    if (user == "root" || user == "daemon" || user == "bin" || 
        user == "sys" || user == "nobody") {
        return true;
    }
#endif
    
    return false;
}

std::string Exs_PlatformUtils::Exs_GetMachineGuid() {
    // Platform-specific implementation needed
    return "";
}

std::string Exs_PlatformUtils::Exs_GetBiosSerialNumber() {
    // Platform-specific implementation needed
    return "";
}

std::string Exs_PlatformUtils::Exs_GetMotherboardSerialNumber() {
    // Platform-specific implementation needed
    return "";
}

std::string Exs_PlatformUtils::Exs_GetDiskSerialNumber() {
    // Platform-specific implementation needed
    return "";
}

bool Exs_PlatformUtils::Exs_IsOnBatteryPower() {
#ifdef _WIN32
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        return status.ACLineStatus == 0;
    }
#endif
    return false;
}

uint32_t Exs_PlatformUtils::Exs_GetBatteryPercentage() {
#ifdef _WIN32
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        return status.BatteryLifePercent;
    }
#endif
    return 100;
}

uint32_t Exs_PlatformUtils::Exs_GetBatteryLifeTime() {
#ifdef _WIN32
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        return status.BatteryLifeTime;
    }
#endif
    return 0;
}

void Exs_PlatformUtils::Exs_RegisterShutdownHandler(void (*handler)(void*), void* context) {
    // Platform-specific implementation needed
}

void Exs_PlatformUtils::Exs_UnregisterShutdownHandler(void (*handler)(void*)) {
    // Platform-specific implementation needed
}

void Exs_PlatformUtils::Exs_DebugBreak() {
#ifdef _WIN32
    DebugBreak();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#else
    // Generate illegal instruction
    __asm__ volatile("ud2");
#endif
}

bool Exs_PlatformUtils::Exs_IsDebuggerPresent() {
#ifdef _WIN32
    return IsDebuggerPresent() != FALSE;
#else
    // Check /proc/self/status on Linux
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.find("TracerPid:") == 0) {
            std::string pidStr = line.substr(10);
            return std::stoi(pidStr) != 0;
        }
    }
    return false;
#endif
}

void Exs_PlatformUtils::Exs_OutputDebugString(const std::string& message) {
#ifdef _WIN32
    OutputDebugStringA(message.c_str());
#else
    std::cerr << "[DEBUG] " << message << std::endl;
#endif
}

uint32_t Exs_PlatformUtils::Exs_GetLastError() {
#ifdef _WIN32
    return GetLastError();
#else
    return errno;
#endif
}

std::string Exs_PlatformUtils::Exs_GetLastErrorString() {
    uint32_t error = Exs_GetLastError();
    std::stringstream ss;
    ss << "Error " << error;
    return ss.str();
}

void Exs_PlatformUtils::Exs_SetLastError(uint32_t error) {
#ifdef _WIN32
    SetLastError(error);
#else
    errno = error;
#endif
}

uint32_t Exs_PlatformUtils::Exs_GetRandomSeed() {
    // Combine various sources for good entropy
    uint32_t seed = 0;
    
    // Current time
    auto now = std::chrono::high_resolution_clock::now();
    seed ^= static_cast<uint32_t>(now.time_since_epoch().count());
    
    // Process ID
    seed ^= Exs_GetProcessId();
    
    // Thread ID
    seed ^= Exs_GetThreadId();
    
    // High-resolution counter
    seed ^= static_cast<uint32_t>(Exs_GetHighResolutionCounter());
    
    // Stack address (for extra entropy)
    int stack_var;
    seed ^= reinterpret_cast<uintptr_t>(&stack_var);
    
    return seed;
}

uint32_t Exs_PlatformUtils::Exs_GenerateRandomNumber() {
    static std::mt19937 rng(Exs_GetRandomSeed());
    return rng();
}

uint64_t Exs_PlatformUtils::Exs_GenerateRandomNumber64() {
    uint64_t high = Exs_GenerateRandomNumber();
    uint64_t low = Exs_GenerateRandomNumber();
    return (high << 32) | low;
}

std::string Exs_PlatformUtils::Exs_GenerateUUID() {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    // Generate random UUID (version 4)
    for (int i = 0; i < 4; i++) {
        ss << std::setw(8) << Exs_GenerateRandomNumber();
    }
    
    std::string uuid = ss.str();
    
    // Format as XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    std::string formatted;
    formatted += uuid.substr(0, 8) + "-";
    formatted += uuid.substr(8, 4) + "-";
    formatted += uuid.substr(12, 4) + "-";
    formatted += uuid.substr(16, 4) + "-";
    formatted += uuid.substr(20, 12);
    
    return formatted;
}

std::string Exs_PlatformUtils::Exs_GenerateGUID() {
    return Exs_GenerateUUID(); // Same format
}

static std::chrono::high_resolution_clock::time_point s_PerformanceStart;

void Exs_PlatformUtils::Exs_BeginPerformanceMeasurement() {
    s_PerformanceStart = std::chrono::high_resolution_clock::now();
}

double Exs_PlatformUtils::Exs_EndPerformanceMeasurement() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(end - s_PerformanceStart);
    return duration.count();
}

bool Exs_PlatformUtils::Exs_HasFeature(const std::string& feature) {
    // Platform-specific feature detection
    return false;
}

std::string Exs_PlatformUtils::Exs_GetFeatureDescription(const std::string& feature) {
    return "Feature not implemented";
}

void Exs_PlatformUtils::Exs_DumpPlatformUtilsInfo() {
    std::cout << "=== Exs Platform Utils Info ===" << std::endl;
    std::cout << "Current Directory: " << Exs_GetCurrentDirectory() << std::endl;
    std::cout << "Home Directory: " << Exs_GetHomeDirectory() << std::endl;
    std::cout << "Temp Directory: " << Exs_GetTempDirectory() << std::endl;
    std::cout << "Executable Path: " << Exs_GetExecutablePath() << std::endl;
    std::cout << "Process ID: " << Exs_GetProcessId() << std::endl;
    std::cout << "Process Name: " << Exs_GetProcessName() << std::endl;
    std::cout << "Computer Name: " << Exs_GetComputerName() << std::endl;
    std::cout << "User Name: " << Exs_GetUserName() << std::endl;
    std::cout << "Is Admin: " << (Exs_IsAdministrator() ? "Yes" : "No") << std::endl;
    std::cout << "Is Debugger Present: " << (Exs_IsDebuggerPresent() ? "Yes" : "No") << std::endl;
    std::cout << "================================" << std::endl;
}

void Exs_PlatformUtils::Exs_RunSelfTests() {
    std::cout << "Running Exs_PlatformUtils self-tests..." << std::endl;
    
    // Test basic functionality
    std::string currentDir = Exs_GetCurrentDirectory();
    std::cout << "Current directory test: " 
              << (!currentDir.empty() ? "PASS" : "FAIL") << std::endl;
    
    uint32_t pid = Exs_GetProcessId();
    std::cout << "Process ID test: " 
              << (pid > 0 ? "PASS" : "FAIL") << std::endl;
    
    std::string computerName = Exs_GetComputerName();
    std::cout << "Computer name test: " 
              << (!computerName.empty() ? "PASS" : "FAIL") << std::endl;
    
    // Test random number generation
    uint32_t r1 = Exs_GenerateRandomNumber();
    uint32_t r2 = Exs_GenerateRandomNumber();
    std::cout << "Random number test: " 
              << (r1 != r2 ? "PASS" : "FAIL") << std::endl;
    
    // Test UUID generation
    std::string uuid = Exs_GenerateUUID();
    std::cout << "UUID generation test: " 
              << (uuid.length() == 36 ? "PASS" : "FAIL") << std::endl;
    
    std::cout << "Self-tests completed." << std::endl;
}

} // namespace Platform
} // namespace Exs
