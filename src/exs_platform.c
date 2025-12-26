#include "exs_platform.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

// Thread-local error buffer
static __thread char exs_last_error[256] = {0};

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <intrin.h>
    #include <Psapi.h>
    #include <malloc.h>
    #pragma comment(lib, "psapi.lib")
    #pragma comment(lib, "kernel32.lib")
#elif defined(__APPLE__)
    #include <mach/mach.h>
    #include <mach/mach_time.h>
    #include <sys/sysctl.h>
    #include <unistd.h>
    #include <dlfcn.h>
    #include <pthread.h>
#elif defined(__linux__) || defined(__unix__)
    #include <unistd.h>
    #include <sys/sysinfo.h>
    #include <sys/auxv.h>
    #include <fcntl.h>
    #include <pthread.h>
    #ifdef __linux__
        #include <cpuid.h>
        #include <malloc.h>
    #endif
#endif

// CPUID implementation
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #ifndef _WIN32
        #ifndef __get_cpuid
            static inline int __exs_get_cpuid(unsigned int level, 
                                             unsigned int* eax, 
                                             unsigned int* ebx,
                                             unsigned int* ecx,
                                             unsigned int* edx) {
                #if defined(__GNUC__) || defined(__clang__)
                    __asm__ volatile ("cpuid"
                                    : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                                    : "a" (level), "c" (0));
                    return 1;
                #else
                    *eax = *ebx = *ecx = *edx = 0;
                    return 0;
                #endif
            }
            #define __get_cpuid __exs_get_cpuid
        #endif
    #endif
#endif

// Cache line size detection
static uint32_t exs_detect_cache_line_size(void) {
    static uint32_t cache_line_size = 0;
    
    if (cache_line_size == 0) {
#ifdef _WIN32
        DWORD buffer_size = 0;
        GetLogicalProcessorInformation(NULL, &buffer_size);
        
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = 
                (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(buffer_size);
            
            if (buffer && GetLogicalProcessorInformation(buffer, &buffer_size)) {
                DWORD offset = 0;
                while (offset < buffer_size) {
                    if (buffer->Relationship == RelationCache && 
                        buffer->Cache.Level == 1) {
                        cache_line_size = buffer->Cache.LineSize;
                        break;
                    }
                    offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                    buffer++;
                }
            }
            free(buffer);
        }
        
        if (cache_line_size == 0) cache_line_size = 64;
        
#elif defined(__APPLE__)
        size_t size = sizeof(cache_line_size);
        if (sysctlbyname("hw.cachelinesize", &cache_line_size, &size, NULL, 0) != 0) {
            cache_line_size = 64;
        }
        
#elif defined(__linux__)
        cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        if (cache_line_size <= 0) cache_line_size = 64;
        
#else
        cache_line_size = 64;
#endif
    }
    
    return cache_line_size;
}

const char* exs_platform_get_name(void) {
    return EXS_PLATFORM_NAME;
}

const char* exs_platform_get_architecture(void) {
    return EXS_ARCH_NAME;
}

const char* exs_platform_get_compiler(void) {
#ifdef EXS_COMPILER_NAME
    return EXS_COMPILER_NAME;
#else
    return "Unknown";
#endif
}

uint32_t exs_platform_get_cpu_count(void) {
    static uint32_t count = 0;
    static int initialized = 0;
    
    if (!initialized) {
#ifdef _WIN32
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        count = sys_info.dwNumberOfProcessors;
        
        // Get physical cores count
        DWORD buffer_size = 0;
        GetLogicalProcessorInformation(NULL, &buffer_size);
        
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = 
                (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(buffer_size);
            
            if (buffer && GetLogicalProcessorInformation(buffer, &buffer_size)) {
                DWORD offset = 0;
                uint32_t physical_cores = 0;
                
                while (offset < buffer_size) {
                    if (buffer->Relationship == RelationProcessorCore) {
                        physical_cores++;
                    }
                    offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                    buffer++;
                }
                
                if (physical_cores > 0) {
                    count = physical_cores;
                }
            }
            free(buffer);
        }
        
#elif defined(__APPLE__)
        int ncpu = 0;
        size_t size = sizeof(ncpu);
        
        // Try to get physical cores first
        if (sysctlbyname("hw.physicalcpu", &ncpu, &size, NULL, 0) == 0 && ncpu > 0) {
            count = (uint32_t)ncpu;
        } else if (sysctlbyname("hw.ncpu", &ncpu, &size, NULL, 0) == 0 && ncpu > 0) {
            count = (uint32_t)ncpu;
        } else {
            count = 1;
        }
        
#elif defined(_SC_NPROCESSORS_ONLN)
        long result = sysconf(_SC_NPROCESSORS_ONLN);
        count = (result > 0) ? (uint32_t)result : 1;
        
#else
        count = 1;
#endif
        
        initialized = 1;
    }
    
    return count;
}

uint64_t exs_platform_get_total_memory(void) {
    static uint64_t memory = 0;
    static int initialized = 0;
    
    if (!initialized) {
#ifdef _WIN32
        MEMORYSTATUSEX mem_info;
        mem_info.dwLength = sizeof(mem_info);
        
        if (GlobalMemoryStatusEx(&mem_info)) {
            memory = mem_info.ullTotalPhys;
        } else {
            DWORD err = GetLastError();
            snprintf(exs_last_error, sizeof(exs_last_error),
                    "GlobalMemoryStatusEx failed: 0x%08lX", err);
            memory = 16ULL * 1024 * 1024 * 1024; // 16GB default
        }
        
#elif defined(__APPLE__)
        int64_t memsize = 0;
        size_t size = sizeof(memsize);
        
        if (sysctlbyname("hw.memsize", &memsize, &size, NULL, 0) == 0) {
            memory = (uint64_t)memsize;
        } else {
            snprintf(exs_last_error, sizeof(exs_last_error),
                    "sysctlbyname failed for hw.memsize");
            memory = 16ULL * 1024 * 1024 * 1024; // 16GB default
        }
        
#elif defined(__linux__)
        struct sysinfo info;
        if (sysinfo(&info) == 0) {
            memory = (uint64_t)info.totalram * (uint64_t)info.mem_unit;
        } else {
            snprintf(exs_last_error, sizeof(exs_last_error),
                    "sysinfo failed: %s", strerror(errno));
            memory = 16ULL * 1024 * 1024 * 1024; // 16GB default
        }
        
#else
        memory = 16ULL * 1024 * 1024 * 1024;
        snprintf(exs_last_error, sizeof(exs_last_error),
                "Total memory detection not implemented for this platform");
#endif
        
        initialized = 1;
    }
    
    return memory;
}

uint64_t exs_platform_get_available_memory(void) {
#ifdef _WIN32
    MEMORYSTATUSEX mem_info;
    mem_info.dwLength = sizeof(mem_info);
    
    if (GlobalMemoryStatusEx(&mem_info)) {
        return mem_info.ullAvailPhys;
    }
    return 0;
    
#elif defined(__APPLE__)
    mach_port_t host_port = MACH_PORT_NULL;
    mach_msg_type_number_t host_size = sizeof(vm_statistics64_data_t) / sizeof(integer_t);
    vm_size_t page_size = 0;
    vm_statistics64_data_t vm_stat = {0};
    kern_return_t kr;
    
    host_port = mach_host_self();
    if (host_port == MACH_PORT_NULL) return 0;
    
    kr = host_page_size(host_port, &page_size);
    if (kr != KERN_SUCCESS) {
        mach_port_deallocate(mach_task_self(), host_port);
        return 0;
    }
    
    kr = host_statistics64(host_port, HOST_VM_INFO64, 
                          (host_info64_t)&vm_stat, &host_size);
    
    mach_port_deallocate(mach_task_self(), host_port);
    
    if (kr == KERN_SUCCESS) {
        uint64_t free_memory = (uint64_t)vm_stat.free_count * (uint64_t)page_size;
        uint64_t inactive_memory = (uint64_t)vm_stat.inactive_count * (uint64_t)page_size;
        return free_memory + inactive_memory;
    }
    return 0;
    
#elif defined(__linux__)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (uint64_t)info.freeram * (uint64_t)info.mem_unit;
    }
    return 0;
    
#else
    return 0;
#endif
}

bool exs_platform_has_sse(void) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#ifdef _WIN32
    int cpu_info[4] = {0};
    __cpuid(cpu_info, 1);
    return (cpu_info[3] & (1 << 25)) != 0;
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return (edx & (1 << 25)) != 0;
    }
#endif
#endif
    return false;
}

bool exs_platform_has_avx(void) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#ifdef _WIN32
    int cpu_info[4] = {0};
    __cpuidex(cpu_info, 1, 0);
    return (cpu_info[2] & (1 << 28)) != 0;
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return (ecx & (1 << 28)) != 0;
    }
#endif
#endif
    return false;
}

bool exs_platform_has_neon(void) {
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    return true;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return true;
#elif defined(__arm__) && defined(__linux__)
#ifdef __linux__
    unsigned long hwcap = getauxval(AT_HWCAP);
#ifdef HWCAP_NEON
    return (hwcap & HWCAP_NEON) != 0;
#endif
#endif
    return false;
#elif defined(__APPLE__) && (defined(__arm__) || defined(__aarch64__))
    return true;
#else
    return false;
#endif
}

void exs_platform_debug_break(void) {
#ifdef _WIN32
    DebugBreak();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#elif defined(__arm__) && !defined(__thumb__)
    __asm__ volatile(".inst 0xe7f000f0");
#elif defined(__aarch64__)
    __asm__ volatile("brk #0");
#else
    raise(SIGTRAP);
#endif
}

void exs_platform_sleep_ms(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    struct timespec req, rem;
    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000;
    
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem;
    }
#endif
}

void exs_platform_sleep_us(uint64_t microseconds) {
#ifdef _WIN32
    HANDLE timer;
    LARGE_INTEGER interval;
    
    interval.QuadPart = -(10 * (LONGLONG)microseconds);
    
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer) {
        SetWaitableTimer(timer, &interval, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
#else
    struct timespec req, rem;
    req.tv_sec = microseconds / 1000000;
    req.tv_nsec = (microseconds % 1000000) * 1000;
    
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem;
    }
#endif
}

const char* exs_platform_get_last_error(void) {
    if (exs_last_error[0] == '\0') {
        return "No error";
    }
    return exs_last_error;
}

void exs_platform_clear_error(void) {
    exs_last_error[0] = '\0';
}

bool exs_platform_is_64bit(void) {
    return sizeof(void*) == 8;
}

bool exs_platform_is_debugger_present(void) {
#ifdef _WIN32
    return IsDebuggerPresent() != FALSE;
#elif defined(__APPLE__)
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
    struct kinfo_proc info = {0};
    size_t size = sizeof(info);
    
    if (sysctl(mib, 4, &info, &size, NULL, 0) == 0) {
        return (info.kp_proc.p_flag & P_TRACED) != 0;
    }
    return false;
#elif defined(__linux__)
    char buf[256] = {0};
    snprintf(buf, sizeof(buf), "/proc/%d/status", getpid());
    
    FILE* f = fopen(buf, "r");
    if (!f) return false;
    
    int tracer_pid = 0;
    char line[256] = {0};
    
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "TracerPid:", 10) == 0) {
            sscanf(line + 10, "%d", &tracer_pid);
            break;
        }
    }
    
    fclose(f);
    return tracer_pid != 0;
#else
    return false;
#endif
}

uint64_t exs_platform_get_tick_count(void) {
#ifdef _WIN32
    return GetTickCount64();
#elif defined(__APPLE__)
    static mach_timebase_info_data_t timebase = {0};
    if (timebase.denom == 0) {
        mach_timebase_info(&timebase);
    }
    return (mach_absolute_time() * timebase.numer) / timebase.denom / 1000000;
#elif defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

uint32_t exs_platform_get_cache_line_size(void) {
    return exs_detect_cache_line_size();
}

uint64_t exs_platform_get_high_res_timer(void) {
#ifdef _WIN32
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#elif defined(__APPLE__)
    return mach_absolute_time();
#elif defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
#endif
}

double exs_platform_get_timer_frequency(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return (double)frequency.QuadPart;
#elif defined(__APPLE__)
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    return 1.0e9 * (double)timebase.numer / (double)timebase.denom;
#elif defined(__linux__)
    return 1.0e9; // nanoseconds per second
#else
    return 1.0e6; // microseconds per second
#endif
}

void* exs_platform_aligned_alloc(size_t size, size_t alignment) {
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#elif defined(__APPLE__) || defined(__linux__)
    void* ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) == 0) {
        return ptr;
    }
    return NULL;
#else
    return malloc(size);
#endif
}

void exs_platform_aligned_free(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#elif defined(__APPLE__) || defined(__linux__)
    free(ptr);
#else
    free(ptr);
#endif
}
