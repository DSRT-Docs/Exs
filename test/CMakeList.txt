cmake_minimum_required(VERSION 3.10)
project(ExsTests)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Include directories
include_directories(../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add main library (if built separately)
# add_subdirectory(../src .)

# C API Test
add_executable(test_basic test_basic.c)
target_link_libraries(test_basic)

# C++ Wrapper Test
add_executable(test_cpp test_cpp.cpp)
target_link_libraries(test_cpp)

# Performance Test
add_executable(test_performance test_performance.c)
target_link_libraries(test_performance)

# Link with platform library (adjust path as needed)
# target_link_libraries(test_basic exs_platform)
# target_link_libraries(test_cpp exs_platform)
# target_link_libraries(test_performance exs_platform)

# Copy source file if needed (simple approach)
if(NOT TARGET exs_platform)
    # Simple approach: compile source directly
    set(PLATFORM_SRC ../src/exs_platform.c)
    
    # Add source files to each test
    target_sources(test_basic PRIVATE ${PLATFORM_SRC})
    target_sources(test_cpp PRIVATE ${PLATFORM_SRC})
    target_sources(test_performance PRIVATE ${PLATFORM_SRC})
endif()

# Add compiler flags
if(MSVC)
    target_compile_options(test_basic PRIVATE /W4)
    target_compile_options(test_cpp PRIVATE /W4)
else()
    target_compile_options(test_basic PRIVATE -Wall -Wextra)
    target_compile_options(test_cpp PRIVATE -Wall -Wextra)
endif()

# Installation (optional)
# install(TARGETS test_basic test_cpp test_performance
#         RUNTIME DESTINATION bin/tests)
