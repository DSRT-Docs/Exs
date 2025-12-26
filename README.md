## Exs

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux%20%7C%20Android%20%7C%20iOS-green.svg)
![C/C++](https://img.shields.io/badge/language-C%2FC%2B%2B-orange.svg)
![CMake](https://img.shields.io/badge/build-CMake-yellow.svg)
![GitHub Release](https://img.shields.io/github/v/release/DSRT-Docs/Exs)
![GitHub Downloads](https://img.shields.io/github/downloads/DSRT-Docs/Exs/total)

**Cross-platform system information library for C and C++**

Selanjutnya: Build System & Packaging

1. CMakeLists.txt (Professional Build System)

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.12)
project(ExsPlatform VERSION 1.0.0 LANGUAGES C CXX)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(EXS_BUILD_TESTS "Build tests" ON)
option(EXS_BUILD_EXAMPLES "Build examples" ON)
option(EXS_BUILD_SHARED "Build shared library" OFF)
option(EXS_BUILD_STATIC "Build static library" ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(EXS_SOURCES
    src/exs_platform.c
    src/exs_platform.cpp
)

# Static library
if(EXS_BUILD_STATIC)
    add_library(exs_platform_static STATIC ${EXS_SOURCES})
    set_target_properties(exs_platform_static PROPERTIES
        OUTPUT_NAME exs_platform
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
    target_include_directories(exs_platform_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Shared library
if(EXS_BUILD_SHARED)
    add_library(exs_platform_shared SHARED ${EXS_SOURCES})
    set_target_properties(exs_platform_shared PROPERTIES
        OUTPUT_NAME exs_platform
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
    target_include_directories(exs_platform_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Tests
if(EXS_BUILD_TESTS)
    enable_testing()
    
    # C API test
    add_executable(test_basic test/platform/test_basic.c)
    target_link_libraries(test_basic exs_platform_static)
    add_test(NAME test_basic COMMAND test_basic)
    
    # C++ API test
    add_executable(test_cpp test/platform/test_cpp.cpp)
    target_link_libraries(test_cpp exs_platform_static)
    add_test(NAME test_cpp COMMAND test_cpp)
    
    # License test
    add_executable(test_license test/platform/test_license.cpp)
    target_link_libraries(test_license exs_platform_static)
    add_test(NAME test_license COMMAND test_license)
    
    # Performance test
    add_executable(test_perf test/platform/test_perf.c)
    target_link_libraries(test_perf exs_platform_static)
    
    # Integration test
    add_executable(test_integration test/platform/test_integration.c)
    target_link_libraries(test_integration exs_platform_static)
    add_test(NAME test_integration COMMAND test_integration)
endif()

# Examples
if(EXS_BUILD_EXAMPLES)
    # Contoh penggunaan sederhana
    add_executable(example_simple examples/simple.c)
    target_link_libraries(example_simple exs_platform_static)
    
    add_executable(example_cpp examples/simple.cpp)
    target_link_libraries(example_cpp exs_platform_static)
endif()

# Installation
install(DIRECTORY include/ DESTINATION include)
install(TARGETS exs_platform_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(EXS_BUILD_SHARED)
    install(TARGETS exs_platform_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

# Package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ExsPlatformConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExsPlatformConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ExsPlatformConfig.cmake
    INSTALL_DESTINATION lib/cmake/ExsPlatform
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ExsPlatformConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ExsPlatformConfigVersion.cmake
    DESTINATION lib/cmake/ExsPlatform
)

# Export targets
export(EXPORT ExsPlatformTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/ExsPlatformTargets.cmake
)

install(EXPORT ExsPlatformTargets
    FILE ExsPlatformTargets.cmake
    DESTINATION lib/cmake/ExsPlatform
)
```

2. cmake/ExsPlatformConfig.cmake.in

```cmake
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/ExsPlatformTargets.cmake")

check_required_components(ExsPlatform)
```

3. .github/workflows/build.yml (CI/CD)

```yaml
name: Build and Test

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        build_type: [Debug, Release]
    
    runs-on: ${{ matrix.os }}
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Configure CMake
      run: |
        cmake -B build \
              -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} \
              -DEXS_BUILD_TESTS=ON \
              -DEXS_BUILD_EXAMPLES=ON
    
    - name: Build
      run: cmake --build build --config ${{ matrix.build_type }}
    
    - name: Test
      run: |
        cd build
        ctest --output-on-failure -C ${{ matrix.build_type }}
    
    - name: Upload artifacts (Windows)
      if: matrix.os == 'windows-latest'
      uses: actions/upload-artifact@v3
      with:
        name: exs-platform-windows-${{ matrix.build_type }}
        path: build/*.lib
    
    - name: Upload artifacts (Linux)
      if: matrix.os == 'ubuntu-latest'
      uses: actions/upload-artifact@v3
      with:
        name: exs-platform-linux-${{ matrix.build_type }}
        path: build/*.a
    
    - name: Upload artifacts (macOS)
      if: matrix.os == 'macos-latest'
      uses: actions/upload-artifact@v3
      with:
        name: exs-platform-macos-${{ matrix.build_type }}
        path: build/*.a
```

4. README.md (Dokumentasi)

```markdown
# Exs Platform Library

[![Build Status](https://github.com/DSRT-Docs/Exs/actions/workflows/build.yml/badge.svg)](https://github.com/DSRT-Docs/Exs/actions)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

Cross-platform system information library written in C with C++ wrapper.

## Features

- **Platform Detection**: Windows, Linux, macOS, Android, iOS, Unix
- **CPU Information**: Core count, features (SSE, AVX, NEON), cache info
- **Memory Information**: Total/available memory, cache line size
- **Timing Functions**: High-resolution timers, sleep functions
- **System Utilities**: Debugger detection, aligned memory allocation
- **Dual API**: Pure C API + Modern C++ wrapper
- **Apache 2.0 Licensed**: Open source with permissive license

## Quick Start

### C API
```c
#include "exs_platform.h"

printf("Platform: %s\n", exs_platform_get_name());
printf("CPU Cores: %u\n", exs_platform_get_cpu_count());
```

C++ API

```cpp
#include "exs_platform.hpp"

std::cout << "Platform: " << exs::Platform::name() << std::endl;
std::cout << "Memory: " << exs::Platform::memory_string() << std::endl;
```

Building

With CMake

```bash
mkdir build && cd build
cmake .. -DEXS_BUILD_TESTS=ON
cmake --build .
```

Manual Compilation

```bash
# C library
gcc -I./include -c src/exs_platform.c -o exs_platform.o

# C++ wrapper
g++ -std=c++11 -I./include -c src/exs_platform.cpp -o exs_platform_cpp.o

# Link
ar rcs libexs_platform.a exs_platform.o exs_platform_cpp.o
```

API Documentation

C API Functions

· exs_platform_get_name() - Platform name
· exs_platform_get_cpu_count() - CPU core count
· exs_platform_get_total_memory() - Total system memory
· exs_platform_has_sse() - SSE support
· exs_platform_sleep_ms() - Sleep milliseconds
· exs_platform_aligned_alloc() - Aligned memory allocation

C++ API

· exs::Platform::name() - Platform name
· exs::Platform::cpu_count() - CPU cores
· exs::Platform::memory_string() - Formatted memory
· exs::Platform::print_info() - Print system info
· exs::Platform::print_license() - Show license

Examples

See examples/ directory:

· simple.c - Basic C usage
· simple.cpp - Basic C++ usage
· system_info.c - Complete system report

Running Tests

```bash
cd build
ctest --output-on-failure

# Or run manually
./test/platform/test_basic
./test/platform/test_cpp
./test/platform/test_integration
```

License

Apache License 2.0 - See LICENSE file for details.

Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

```

## **5. `examples/` Directory**

```

examples/
├──simple.c        ← C API example
├──simple.cpp      ← C++ API example
└──system_info.c   ← Complete system report
