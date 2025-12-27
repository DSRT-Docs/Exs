#pragma once
#include <cstdint>

namespace Exs {
namespace Platform {

class Exs_Platform {
private:
    static bool s_Initialized;
    static uint64_t s_StartupTime;
    
public:
    // Core platform functions
    static void Exs_Initialize();
    static void Exs_Shutdown();
    
    // Platform information
    static const char* Exs_GetPlatformName();
    static const char* Exs_GetVersionString();
    
    // Internal utilities
    static uint64_t Exs_GetInternalTimestamp();
    static bool Exs_IsInitialized();
    
    // Debug functions
    static void Exs_DumpPlatformInfo();
};

} // namespace Platform
} // namespace Exs
