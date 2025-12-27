// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once
#include <cstdint>

namespace Exs {
namespace Platform {

/// Core platform initialization and management
class Exs_Platform {
private:
    static bool s_Initialized;          ///< Platform initialization flag
    static uint64_t s_StartupTime;      ///< Platform startup timestamp (ms)
    
public:
    /// @brief Initialize platform subsystems
    /// @note Must be called before any other platform functions
    static void Exs_Initialize();
    
    /// @brief Shutdown platform subsystems
    static void Exs_Shutdown();
    
    /// @brief Get platform name
    /// @return Platform name string (static, do not free)
    static const char* Exs_GetPlatformName() noexcept;
    
    /// @brief Get platform version string
    /// @return Version string (static, do not free)
    static const char* Exs_GetVersionString() noexcept;
    
    /// @brief Get internal timestamp for platform operations
    /// @return Timestamp in milliseconds
    static uint64_t Exs_GetInternalTimestamp() noexcept;
    
    /// @brief Check if platform is initialized
    /// @return true if platform is initialized
    static bool Exs_IsInitialized() noexcept;
    
    /// @brief Dump platform information to debug output
    static void Exs_DumpPlatformInfo();
};

} // namespace Platform
} // namespace Exs
