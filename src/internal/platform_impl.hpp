#ifndef EXS_PLATFORM_IMPL_HPP
#define EXS_PLATFORM_IMPL_HPP

#include "cache.hpp"

namespace exs {
namespace internal {

class PlatformImpl {
public:
    static void initialize_cache() {
        auto& cache = Cache::get_instance();
        if (!cache.is_initialized()) {
            // Values akan di-set dari exs_platform.cpp
            cache.mark_initialized();
        }
    }
    
    static uint32_t get_cached_cpu_count() {
        return Cache::get_instance().get_cpu_count();
    }
    
    static uint64_t get_cached_total_memory() {
        return Cache::get_instance().get_total_memory();
    }
    
    static uint32_t get_cached_cache_line_size() {
        return Cache::get_instance().get_cache_line_size();
    }
    
    static void update_cache_values(uint32_t cpu_count, 
                                   uint64_t total_memory,
                                   uint32_t cache_line_size) {
        auto& cache = Cache::get_instance();
        cache.set_cpu_count(cpu_count);
        cache.set_total_memory(total_memory);
        cache.set_cache_line_size(cache_line_size);
    }
};

} // namespace internal
} // namespace exs

#endif // EXS_PLATFORM_IMPL_HPP
