#include <iostream>
#include "exs_platform.hpp"

int main() {
    std::cout << "System Information:" << std::endl;
    std::cout << "Platform: " << exs::Platform::name() << std::endl;
    std::cout << "CPU Cores: " << exs::Platform::cpu_count() << std::endl;
    std::cout << "Memory: " << exs::Platform::memory_string() << std::endl;
    return 0;
}
