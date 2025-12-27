// src/Core/Platform/internal/PlatformBase.h
#ifndef EXS_INTERNAL_PLATFORM_BASE_H
#define EXS_INTERNAL_PLATFORM_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>

namespace Exs {
namespace Internal {
namespace Platform {

// Platform type enumeration
enum class Exs_PlatformType {
    Unknown = 0,
    Windows = 1,
    Linux = 2,
    MacOS = 3,
    Android = 4,
    iOS = 5,
    PlayStation = 6,
    Xbox = 7,
    Switch = 8
};

// Architecture enumeration
enum class Exs_Architecture {
    Unknown = 0,
    x86 = 1,
    x64 = 2,
    ARM = 3,
    ARM64 = 4,
    MIPS = 5,
    PowerPC = 6
};

// Endianness enumeration
enum class Exs_Endianness {
    Unknown = 0,
    Little = 1,
    Big = 2
};

// Base platform class
class Exs_PlatformBase {
public:
    virtual ~Exs_PlatformBase() = default;
    
    // Platform detection
    virtual Exs_PlatformType getPlatformType() const = 0;
    virtual Exs_Architecture getArchitecture() const = 0;
    virtual Exs_Endianness getEndianness() const = 0;
    
    // Platform information
    virtual std::string getPlatformName() const = 0;
    virtual std::string getPlatformVersion() const = 0;
    virtual std::string getPlatformVendor() const = 0;
    
    // System capabilities
    virtual bool isMobilePlatform() const = 0;
    virtual bool isDesktopPlatform() const = 0;
    virtual bool isConsolePlatform() const = 0;
    
    // Feature detection
    virtual bool supportsSIMD() const = 0;
    virtual bool supportsAVX() const = 0;
    virtual bool supportsNEON() const = 0;
    
    // System paths
    virtual std::string getHomeDirectory() const = 0;
    virtual std::string getTempDirectory() const = 0;
    virtual std::string getAppDataDirectory() const = 0;
    virtual std::string getExecutableDirectory() const = 0;
    
    // System commands
    virtual void executeCommand(const std::string& command) const = 0;
    virtual int32 executeCommandWithResult(const std::string& command) const = 0;
    
    // Thread and process
    virtual uint32 getCurrentThreadId() const = 0;
    virtual uint32 getCurrentProcessId() const = 0;
    
    // Timing
    virtual uint64 getHighResolutionTimer() const = 0;
    virtual double getHighResolutionTimerFrequency() const = 0;
    
    // Memory page size
    virtual uint32 getMemoryPageSize() const = 0;
    
    // CPU count
    virtual uint32 getPhysicalCoreCount() const = 0;
    virtual uint32 getLogicalCoreCount() const = 0;
};

// Factory function
Exs_PlatformBase* Exs_CreatePlatformInstance();

} // namespace Platform
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_PLATFORM_BASE_H
