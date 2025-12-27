#pragma once
#include <cstdint>
#include <string>

namespace Exs {
namespace Platform {

class Exs_PlatformUtils {
public:
    // File system utilities
    static std::string Exs_GetCurrentDirectory();
    static bool Exs_SetCurrentDirectory(const std::string& path);
    static std::string Exs_GetHomeDirectory();
    static std::string Exs_GetTempDirectory();
    static std::string Exs_GetAppDataDirectory();
    static std::string Exs_GetExecutablePath();
    static std::string Exs_GetModuleDirectory();
    
    // Process utilities
    static uint32_t Exs_GetProcessId();
    static uint32_t Exs_GetParentProcessId();
    static uint32_t Exs_GetThreadId();
    static std::string Exs_GetProcessName();
    static uint64_t Exs_GetProcessStartTime();
    
    // System utilities
    static void Exs_Sleep(uint32_t milliseconds);
    static void Exs_YieldProcessor();
    static uint64_t Exs_GetTickCount();  // Milliseconds since system start
    static uint64_t Exs_GetHighResolutionCounter();
    static uint64_t Exs_GetHighResolutionFrequency();
    static double Exs_GetSystemTime();  // Seconds with high precision
    
    // Console utilities
    static bool Exs_IsConsoleAttached();
    static void Exs_WriteToConsole(const std::string& text);
    static void Exs_WriteErrorToConsole(const std::string& text);
    static void Exs_SetConsoleColor(uint32_t color);
    static void Exs_ResetConsoleColor();
    static bool Exs_IsStdoutRedirected();
    static bool Exs_IsStderrRedirected();
    
    // Environment utilities
    static std::string Exs_GetEnvironmentVariable(const std::string& name);
    static bool Exs_SetEnvironmentVariable(const std::string& name, const std::string& value);
    static bool Exs_HasEnvironmentVariable(const std::string& name);
    static void Exs_DumpEnvironmentVariables();
    
    // System commands
    static int Exs_ExecuteCommand(const std::string& command, std::string* output = nullptr);
    static int Exs_ExecuteCommandWithTimeout(const std::string& command, 
                                           uint32_t timeout_ms,
                                           std::string* output = nullptr);
    
    // System information
    static std::string Exs_GetComputerName();
    static std::string Exs_GetUserName();
    static std::string Exs_GetDomainName();
    static bool Exs_IsAdministrator();
    static bool Exs_IsServiceAccount();
    
    // Hardware information
    static std::string Exs_GetMachineGuid();
    static std::string Exs_GetBiosSerialNumber();
    static std::string Exs_GetMotherboardSerialNumber();
    static std::string Exs_GetDiskSerialNumber();
    
    // System power
    static bool Exs_IsOnBatteryPower();
    static uint32_t Exs_GetBatteryPercentage();
    static uint32_t Exs_GetBatteryLifeTime();  // Seconds remaining
    
    // System events
    static void Exs_RegisterShutdownHandler(void (*handler)(void*), void* context);
    static void Exs_UnregisterShutdownHandler(void (*handler)(void*));
    
    // Debug utilities
    static void Exs_DebugBreak();
    static bool Exs_IsDebuggerPresent();
    static void Exs_OutputDebugString(const std::string& message);
    
    // Error handling
    static uint32_t Exs_GetLastError();
    static std::string Exs_GetLastErrorString();
    static void Exs_SetLastError(uint32_t error);
    
    // Random utilities
    static uint32_t Exs_GetRandomSeed();
    static uint32_t Exs_GenerateRandomNumber();
    static uint64_t Exs_GenerateRandomNumber64();
    
    // UUID generation
    static std::string Exs_GenerateUUID();
    static std::string Exs_GenerateGUID();
    
    // Performance utilities
    static void Exs_BeginPerformanceMeasurement();
    static double Exs_EndPerformanceMeasurement();  // Returns seconds
    
    // Platform-specific features
    static bool Exs_HasFeature(const std::string& feature);
    static std::string Exs_GetFeatureDescription(const std::string& feature);
    
    // Debug functions
    static void Exs_DumpPlatformUtilsInfo();
    static void Exs_RunSelfTests();
};

} // namespace Platform
} // namespace Exs
