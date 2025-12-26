#ifndef EXS_CACHE_HPP
#define EXS_CACHE_HPP

#include <atomic>
#include <mutex>
#include <string>
#include <cstdint>

namespace exs {
namespace internal {

class Cache {
private:
    std::atomic<uint32_t> cpu_count{0};
    std::atomic<uint64_t> total_memory{0};
    std::atomic<uint32_t> cache_line_size{0};
    std::atomic<bool> initialized{false};
    std::mutex init_mutex;
    
public:
    static Cache& get_instance() {
        static Cache instance;
        return instance;
    }
    
    uint32_t get_cpu_count() {
        ensure_initialized();
        return cpu_count.load(std::memory_order_acquire);
    }
    
    uint64_t get_total_memory() {
        ensure_initialized();
        return total_memory.load(std::memory_order_acquire);
    }
    
    uint32_t get_cache_line_size() {
        ensure_initialized();
        return cache_line_size.load(std::memory_order_acquire);
    }
    
    void set_cpu_count(uint32_t count) {
        std::lock_guard<std::mutex> lock(init_mutex);
        cpu_count.store(count, std::memory_order_release);
    }
    
    void set_total_memory(uint64_t memory) {
        std::lock_guard<std::mutex> lock(init_mutex);
        total_memory.store(memory, std::memory_order_release);
    }
    
    void set_cache_line_size(uint32_t size) {
        std::lock_guard<std::mutex> lock(init_mutex);
        cache_line_size.store(size, std::memory_order_release);
    }
    
    void mark_initialized() {
        std::lock_guard<std::mutex> lock(init_mutex);
        initialized.store(true, std::memory_order_release);
    }
    
    bool is_initialized() const {
        return initialized.load(std::memory_order_acquire);
    }
    
private:
    Cache() = default;
    
    void ensure_initialized() {
        if (!initialized.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(init_mutex);
            if (!initialized.load(std::memory_order_relaxed)) {
                cpu_count.store(0, std::memory_order_release);
                total_memory.store(0, std::memory_order_release);
                cache_line_size.store(0, std::memory_order_release);
                initialized.store(true, std::memory_order_release);
            }
        }
    }
};

} // namespace internal
} // namespace exs

#endif // EXS_CACHE_HPP
