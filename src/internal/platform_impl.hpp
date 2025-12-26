#ifndef EXS_PLATFORM_IMPL_HPP
#define EXS_PLATFORM_IMPL_HPP

#include <string>
#include <cstdint>
#include <atomic>
#include <mutex>

namespace exs {
namespace internal {

struct PlatformCache {
    std::atomic<uint32_t> cpu_count{0};
    std::atomic<uint64_t> total_memory{0};
    std::atomic<bool> initialized{false};
    std::mutex init_mutex;
    
    void initialize() {
        if (!initialized.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(init_mutex);
            if (!initialized.load(std::memory_order_relaxed)) {
                // Initialize values
                cpu_count.store(0, std::memory_order_release);
                total_memory.store(0, std::memory_order_release);
                initialized.store(true, std::memory_order_release);
            }
        }
    }
};

class PlatformImpl {
private:
    static PlatformCache& get_cache() {
        static PlatformCache cache;
        return cache;
    }
    
public:
    static uint32_t get_cpu_count_cached() {
        auto& cache = get_cache();
        cache.initialize();
        return cache.cpu_count.load(std::memory_order_acquire);
    }
    
    static uint64_t get_total_memory_cached() {
        auto& cache = get_cache();
        cache.initialize();
        return cache.total_memory.load(std::memory_order_acquire);
    }
    
    static void update_cache(uint32_t cpu_count, uint64_t total_memory) {
        auto& cache = get_cache();
        std::lock_guard<std::mutex> lock(cache.init_mutex);
        cache.cpu_count.store(cpu_count, std::memory_order_release);
        cache.total_memory.store(total_memory, std::memory_order_release);
        cache.initialized.store(true, std::memory_order_release);
    }
};

} // namespace internal
} // namespace exs

#endif // EXS_PLATFORM_IMPL_HPP
