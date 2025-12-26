/*
 * Copyright [2024] [DSRT-Docs]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <iostream>
#include <cstring>
#include "../../include/exs_platform.hpp"

int main() {
    std::cout << "=== Exs Platform License Test ===\n\n";
    
    // Test 1: Print license
    std::cout << "Testing print_license()...\n";
    exs::Platform::print_license();
    
    // Test 2: Get license text
    std::cout << "\nTesting license_text()...\n";
    const char* license = exs::Platform::license_text();
    
    if (license && strlen(license) > 0) {
        std::cout << "✓ License text available (" << strlen(license) << " chars)\n";
        
        // Check for Apache 2.0 keywords
        if (strstr(license, "Apache License") != nullptr &&
            strstr(license, "Version 2.0") != nullptr) {
            std::cout << "✓ Apache 2.0 License confirmed\n";
        } else {
            std::cout << "⚠ Warning: May not be Apache 2.0 text\n";
        }
    } else {
        std::cout << "✗ License text not available\n";
        return 1;
    }
    
    // Test 3: Check print_info includes license
    std::cout << "\nChecking print_info() license notice...\n";
    // Note: print_info() will print to console, we just verify it doesn't crash
    exs::Platform::print_info();
    
    std::cout << "\n=== License Tests Completed ===\n";
    return 0;
}
