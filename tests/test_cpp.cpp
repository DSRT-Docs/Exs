#include <iostream>
#include <cassert>
#include "../include/exs_platform.hpp"

void test_cpp_basic() {
    std::cout << "=== Testing C++ Wrapper (Basic) ===" << std::endl;
    
    std::string name = exs::Platform::name();
    std::string arch = exs::Platform::architecture();
    std::string compiler = exs::Platform::compiler();
    
    std::cout << "Platform: " << name << std::endl;
    std::cout << "Architecture: " << arch << std::endl;
    std::cout << "Compiler: " << compiler << std::endl;
    
    assert(!name.empty());
    assert(!arch.empty());
    assert(!compiler.empty());
    
    std::cout << "✓ C++ basic test passed\n" << std::endl;
}

void test_cpp_memory_string() {
    std::cout << "=== Testing C++ Memory Formatting ===" << std::endl;
    
    std::string mem_str = exs::Platform::memory_string(true);
    std::string mem_raw = exs::Platform::memory_string(false);
    
    std::cout << "Human readable: " << mem_str << std::endl;
    std::cout << "Raw: " << mem_raw << std::endl;
    
    assert(!mem_str.empty());
    assert(!mem_raw.empty());
    
    // Test percentage
    double usage = exs::Platform::memory_usage_percentage();
    std::cout << "Memory usage: " << usage << "%" << std::endl;
    
    assert(usage >= 0.0 && usage <= 100.0);
    
    std::cout << "✓ C++ memory formatting test passed\n" << std::endl;
}

void test_cpp_platform_checks() {
    std::cout << "=== Testing C++ Platform Checks ===" << std::endl;
    
    bool is_windows = exs::Platform::is_windows();
    bool is_linux = exs::Platform::is_linux();
    bool is_macos = exs::Platform::is_macos();
    bool is_desktop = exs::Platform::is_desktop();
    bool is_mobile = exs::Platform::is_mobile();
    bool is_unix = exs::Platform::is_unix();
    
    std::cout << "Is Windows: " << (is_windows ? "Yes" : "No") << std::endl;
    std::cout << "Is Linux: " << (is_linux ? "Yes" : "No") << std::endl;
    std::cout << "Is macOS: " << (is_macos ? "Yes" : "No") << std::endl;
    std::cout << "Is Desktop: " << (is_desktop ? "Yes" : "No") << std::endl;
    std::cout << "Is Mobile: " << (is_mobile ? "Yes" : "No") << std::endl;
    std::cout << "Is Unix: " << (is_unix ? "Yes" : "No") << std::endl;
    
    // Platform type
    exs::PlatformType type = exs::Platform::type();
    const char* type_str = "Unknown";
    switch(type) {
        case exs::PlatformType::Desktop: type_str = "Desktop"; break;
        case exs::PlatformType::Mobile: type_str = "Mobile"; break;
        case exs::PlatformType::Server: type_str = "Server"; break;
        case exs::PlatformType::Embedded: type_str = "Embedded"; break;
        default: type_str = "Unknown";
    }
    std::cout << "Platform Type: " << type_str << std::endl;
    
    std::cout << "✓ C++ platform checks test passed\n" << std::endl;
}

void test_cpp_aligned_memory() {
    std::cout << "=== Testing C++ Aligned Memory ===" << std::endl;
    
    size_t size = 1024;
    size_t alignment = 64;
    
    void* ptr = exs::Platform::aligned_alloc(size, alignment);
    assert(ptr != nullptr);
    
    bool is_aligned = exs::Platform::is_aligned(ptr, alignment);
    std::cout << "Allocated " << size << " bytes at " << ptr 
              << " (aligned to " << alignment << "): " 
              << (is_aligned ? "Yes" : "No") << std::endl;
    
    assert(is_aligned);
    
    exs::Platform::aligned_free(ptr);
    
    std::cout << "✓ C++ aligned memory test passed\n" << std::endl;
}

void test_cpp_print_info() {
    std::cout << "=== Testing C++ Print Info ===" << std::endl;
    
    // Capture output (simplified)
    std::cout << "Calling print_info():" << std::endl;
    exs::Platform::print_info();
    
    std::cout << "✓ C++ print info test passed\n" << std::endl;
}

void test_cpp_error_handling() {
    std::cout << "=== Testing C++ Error Handling ===" << std::endl;
    
    std::string error = exs::Platform::last_error();
    bool has_error = exs::Platform::has_error();
    
    std::cout << "Last error: " << error << std::endl;
    std::cout << "Has error: " << (has_error ? "Yes" : "No") << std::endl;
    
    exs::Platform::clear_error();
    error = exs::Platform::last_error();
    has_error = exs::Platform::has_error();
    
    std::cout << "After clear - Last error: " << error << std::endl;
    std::cout << "After clear - Has error: " << (has_error ? "Yes" : "No") << std::endl;
    
    std::cout << "✓ C++ error handling test passed\n" << std::endl;
}

int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "Exs C++ Platform Wrapper Test Suite" << std::endl;
    std::cout << "=====================================\n" << std::endl;
    
    try {
        test_cpp_basic();
        test_cpp_memory_string();
        test_cpp_platform_checks();
        test_cpp_aligned_memory();
        test_cpp_print_info();
        test_cpp_error_handling();
        
        std::cout << "=====================================" << std::endl;
        std::cout << "ALL C++ TESTS PASSED SUCCESSFULLY!" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
