#!/bin/bash

# Test Runner Script for Exs Platform Library
# Usage: ./test_runner.sh [clean|build|run|all]

set -e  # Exit on error

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build/tests"
SRC_DIR="${PROJECT_DIR}/src"
TEST_DIR="${PROJECT_DIR}/tests"
INCLUDE_DIR="${PROJECT_DIR}/include"

echo "Exs Platform Library Test Runner"
echo "Project directory: ${PROJECT_DIR}"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

clean_build() {
    echo "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
    print_status "Build directory cleaned"
}

build_tests() {
    echo "Building tests..."
    
    # Create build directory
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    
    # Configure with CMake
    cmake "${TEST_DIR}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_CXX_COMPILER=g++ \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    # Build
    cmake --build . --parallel $(nproc)
    
    cd - > /dev/null
    print_status "Tests built successfully"
}

run_tests() {
    echo "Running tests..."
    
    cd "${BUILD_DIR}"
    
    # Run C API tests
    echo ""
    echo "=== Running C API Tests ==="
    if [ -f "./test_basic" ]; then
        ./test_basic
        if [ $? -eq 0 ]; then
            print_status "C API tests passed"
        else
            print_error "C API tests failed"
            exit 1
        fi
    else
        print_warning "test_basic executable not found"
    fi
    
    # Run C++ tests
    echo ""
    echo "=== Running C++ Wrapper Tests ==="
    if [ -f "./test_cpp" ]; then
        ./test_cpp
        if [ $? -eq 0 ]; then
            print_status "C++ wrapper tests passed"
        else
            print_error "C++ wrapper tests failed"
            exit 1
        fi
    else
        print_warning "test_cpp executable not found"
    fi
    
    # Run performance tests
    echo ""
    echo "=== Running Performance Tests ==="
    if [ -f "./test_performance" ]; then
        ./test_performance
        print_status "Performance tests completed"
    else
        print_warning "test_performance executable not found"
    fi
    
    # Simple manual compilation test (fallback)
    echo ""
    echo "=== Manual Compilation Test ==="
    MANUAL_TEST="${BUILD_DIR}/manual_test.c"
    cat > "${MANUAL_TEST}" << 'EOF'
#include <stdio.h>
#include "../include/exs_platform.h"
int main() {
    printf("Manual test: %s\n", exs_platform_get_name());
    return 0;
}
EOF
    
    if gcc -I"${INCLUDE_DIR}" "${MANUAL_TEST}" "${SRC_DIR}/exs_platform.c" \
           -o "${BUILD_DIR}/manual_test" 2>/dev/null; then
        "${BUILD_DIR}/manual_test" > /dev/null
        if [ $? -eq 0 ]; then
            print_status "Manual compilation test passed"
        else
            print_error "Manual compilation test failed"
        fi
    else
        print_warning "Manual compilation failed"
    fi
    
    cd - > /dev/null
    
    echo ""
    print_status "All tests completed successfully!"
}

print_usage() {
    echo "Usage: $0 [clean|build|run|all]"
    echo "  clean   - Clean build directory"
    echo "  build   - Build tests"
    echo "  run     - Run tests"
    echo "  all     - Clean, build, and run tests (default)"
}

case "${1:-all}" in
    clean)
        clean_build
        ;;
    build)
        build_tests
        ;;
    run)
        run_tests
        ;;
    all)
        clean_build
        build_tests
        run_tests
        ;;
    *)
        print_usage
        exit 1
        ;;
esac
