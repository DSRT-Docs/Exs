# src/Core/Platform/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(ExsPlatformInternal)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    set(EXS_PLATFORM_WINDOWS ON)
    set(EXS_PLATFORM_NAME "Windows")
    add_definitions(-DEXS_PLATFORM_WINDOWS)
elseif(APPLE)
    set(EXS_PLATFORM_MAC ON)
    set(EXS_PLATFORM_NAME "macOS")
    add_definitions(-DEXS_PLATFORM_MAC)
elseif(UNIX)
    set(EXS_PLATFORM_LINUX ON)
    set(EXS_PLATFORM_NAME "Linux")
    add_definitions(-DEXS_PLATFORM_LINUX)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Internal headers
set(INTERNAL_HEADERS
    internal/PlatformBase.h
    internal/SystemInfoBase.h
    internal/CPUInfoBase.h
    internal/MemoryInfoBase.h
    internal/FileSystemBase.h
    internal/NetworkInfoBase.h
    internal/GraphicsInfoBase.h
    internal/AudioInfoBase.h
    internal/InputInfoBase.h
    internal/BIOSInfoBase.h
    internal/SecurityInfoBase.h
    internal/PowerInfoBase.h
    internal/PerformanceInfoBase.h
)

# Platform-specific source files
if(EXS_PLATFORM_WINDOWS)
    set(PLATFORM_SOURCES
        Windows/PlatformWindows.cpp
        Windows/SystemInfoWindows.cpp
        Windows/CPUInfoWindows.cpp
        Windows/MemoryInfoWindows.cpp
        Windows/FileSystemWindows.cpp
        Windows/NetworkInfoWindows.cpp
        Windows/GraphicsInfoWindows.cpp
        Windows/AudioInfoWindows.cpp
        Windows/InputInfoWindows.cpp
        Windows/BIOSInfoWindows.cpp
        Windows/SecurityInfoWindows.cpp
        Windows/PowerInfoWindows.cpp
        Windows/PerformanceInfoWindows.cpp
    )
    
    # Windows-specific libraries
    set(PLATFORM_LIBRARIES
        kernel32.lib
        user32.lib
        gdi32.lib
        winspool.lib
        comdlg32.lib
        advapi32.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
        uuid.lib
        odbc32.lib
        odbccp32.lib
        setupapi.lib
        wbemuuid.lib
        powrprof.lib
        iphlpapi.lib
        winmm.lib
        dsound.lib
        dxgi.lib
        d3d12.lib
        d3d11.lib
        d3d9.lib
        dxguid.lib
    )
    
elseif(EXS_PLATFORM_LINUX)
    set(PLATFORM_SOURCES
        Linux/PlatformLinux.cpp
        Linux/SystemInfoLinux.cpp
        Linux/CPUInfoLinux.cpp
        Linux/MemoryInfoLinux.cpp
        Linux/FileSystemLinux.cpp
        Linux/NetworkInfoLinux.cpp
        Linux/GraphicsInfoLinux.cpp
        Linux/AudioInfoLinux.cpp
        Linux/InputInfoLinux.cpp
        Linux/BIOSInfoLinux.cpp
        Linux/SecurityInfoLinux.cpp
        Linux/PowerInfoLinux.cpp
        Linux/PerformanceInfoLinux.cpp
    )
    
    # Linux-specific libraries
    set(PLATFORM_LIBRARIES
        pthread
        dl
        rt
        m
        X11
        Xi
        Xrandr
        Xxf86vm
        Xinerama
        Xcursor
        alsa
        pulse
        pulse-simple
        udev
        GL
        GLU
    )
    
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
    pkg_check_modules(PULSEAUDIO REQUIRED libpulse)
    pkg_check_modules(PULSEAUDIO_SIMPLE REQUIRED libpulse-simple)
    
elseif(EXS_PLATFORM_MAC)
    set(PLATFORM_SOURCES
        Mac/PlatformMac.cpp
        Mac/SystemInfoMac.cpp
        Mac/CPUInfoMac.cpp
        Mac/MemoryInfoMac.cpp
        Mac/FileSystemMac.cpp
        Mac/NetworkInfoMac.cpp
        Mac/GraphicsInfoMac.cpp
        Mac/AudioInfoMac.cpp
        Mac/InputInfoMac.cpp
        Mac/BIOSInfoMac.cpp
        Mac/SecurityInfoMac.cpp
        Mac/PowerInfoMac.cpp
        Mac/PerformanceInfoMac.cpp
    )
    
    # macOS-specific frameworks
    set(PLATFORM_LIBRARIES
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreAudio"
        "-framework CoreVideo"
        "-framework QuartzCore"
        "-framework OpenGL"
        "-framework Metal"
        "-framework Security"
        "-framework SystemConfiguration"
    )
endif()

# Create library
add_library(ExsPlatformInternal STATIC
    ${INTERNAL_HEADERS}
    ${PLATFORM_SOURCES}
)

# Include directories
target_include_directories(ExsPlatformInternal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
)

# Link libraries
target_link_libraries(ExsPlatformInternal PRIVATE
    ${PLATFORM_LIBRARIES}
)

# Compiler options
target_compile_options(ExsPlatformInternal PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /MP>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# Install internal headers (development only)
install(DIRECTORY internal/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Exs/Core/Platform/internal
    FILES_MATCHING PATTERN "*.h"
)
