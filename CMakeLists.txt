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
