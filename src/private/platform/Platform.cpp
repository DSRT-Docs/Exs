#include "Platform.h"
#include "../Debug/Assert.h"
#include "../Debug/DebugLogger.h"
#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined(EXS_PLATFORM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #include <intrin.h>
    #include <tlhelp32.h>
    #include <psapi.h>
    #include <shlobj.h>
    #include <shellapi.h>
    #include <direct.h>
    #include <io.h>
    #include <fcntl.h>
    #pragma comment(lib, "psapi.lib")
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "advapi32.lib")
#elif defined(EXS_PLATFORM_LINUX) || defined(EXS_PLATFORM_APPLE)
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <pthread.h>
    #include <dlfcn.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <signal.h>
    #include <sched.h>
    #include <libgen.h>
    #ifdef EXS_PLATFORM_LINUX
        #include <sys/syscall.h>
        #include <linux/limits.h>
        #include <malloc.h>
        #include <sys/sysinfo.h>
    #elif defined(EXS_PLATFORM_APPLE)
        #include <mach/mach.h>
        #include <mach/mach_time.h>
        #include <mach-o/dyld.h>
        #include <CoreServices/CoreServices.h>
        #include <libproc.h>
    #endif
#endif

namespace Exs {
namespace Internal {
namespace Platform {

// Static member initialization
bool Exs_Platform::Exs_initialized = false;
Exs_uint64 Exs_Platform::Exs_performanceCounterFrequency = 0;
double Exs_Platform::Exs_secondsPerCycle = 0.0;

// Initialization and shutdown
void Exs_Platform::Exs_Initialize() {
    if (Exs_initialized) {
        return;
    }
    
    // Initialize performance counter
#if defined(EXS_PLATFORM_WINDOWS)
    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency)) {
        Exs_performanceCounterFrequency = frequency.QuadPart;
        Exs_secondsPerCycle = 1.0 / static_cast<double>(frequency.QuadPart);
    } else {
        Exs_performanceCounterFrequency = 1000; // Fallback to GetTickCount
        Exs_secondsPerCycle = 0.001;
    }
#elif defined(EXS_PLATFORM_APPLE)
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    Exs_performanceCounterFrequency = 1000000000ULL * static_cast<Exs_uint64>(timebase.denom) / static_cast<Exs_uint64>(timebase.numer);
    Exs_secondsPerCycle = static_cast<double>(timebase.numer) / static_cast<double>(timebase.denom) / 1000000000.0;
#else
    // Linux and others use CLOCK_MONOTONIC
    Exs_performanceCounterFrequency = 1000000000ULL; // nanoseconds per second
    Exs_secondsPerCycle = 1e-9;
#endif
    
    Exs_initialized = true;
    
    // Initialize subsystems
    Exs_SystemInfo::Exs_Initialize();
    Exs_CPUInfo::Exs_Initialize();
    Exs_MemoryInfo::Exs_Initialize();
    
    EXS_LOG_INFO("Platform initialized");
}

void Exs_Platform::Exs_Shutdown() {
    if (!Exs_initialized) {
        return;
    }
    
    // Shutdown subsystems
    Exs_MemoryInfo::Exs_Shutdown();
    Exs_CPUInfo::Exs_Shutdown();
    Exs_SystemInfo::Exs_Shutdown();
    
    Exs_initialized = false;
    EXS_LOG_INFO("Platform shutdown");
}

// Time functions
Exs_uint64 Exs_Platform::Exs_GetPerformanceCounter() {
#if defined(EXS_PLATFORM_WINDOWS)
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#elif defined(EXS_PLATFORM_APPLE)
    return mach_absolute_time();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<Exs_uint64>(ts.tv_sec) * 1000000000ULL + static_cast<Exs_uint64>(ts.tv_nsec);
#endif
}

Exs_uint64 Exs_Platform::Exs_GetPerformanceFrequency() {
    return Exs_performanceCounterFrequency;
}

double Exs_Platform::Exs_GetTime() {
    return static_cast<double>(Exs_GetPerformanceCounter()) * Exs_secondsPerCycle;
}

double Exs_Platform::Exs_GetHighPrecisionTime() {
    return Exs_GetTime();
}

// Sleep and yield
void Exs_Platform::Exs_Sleep(Exs_uint32 milliseconds) {
#if defined(EXS_PLATFORM_WINDOWS)
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, nullptr);
#endif
}

void Exs_Platform::Exs_SleepMicroseconds(Exs_uint32 microseconds) {
#if defined(EXS_PLATFORM_WINDOWS)
    // Windows doesn't have microsecond sleep, use high-resolution timer
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    
    Exs_uint64 ticksToWait = static_cast<Exs_uint64>(microseconds) * frequency.QuadPart / 1000000ULL;
    
    do {
        QueryPerformanceCounter(&end);
    } while ((end.QuadPart - start.QuadPart) < ticksToWait);
#else
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000L;
    nanosleep(&ts, nullptr);
#endif
}

void Exs_Platform::Exs_SleepNanoseconds(Exs_uint64 nanoseconds) {
#if defined(EXS_PLATFORM_WINDOWS)
    // Windows precision is limited, use microseconds approximation
    Exs_SleepMicroseconds(static_cast<Exs_uint32>(nanoseconds / 1000));
#else
    struct timespec ts;
    ts.tv_sec = nanoseconds / 1000000000ULL;
    ts.tv_nsec = nanoseconds % 1000000000ULL;
    nanosleep(&ts, nullptr);
#endif
}

void Exs_Platform::Exs_YieldThread() {
#if defined(EXS_PLATFORM_WINDOWS)
    SwitchToThread();
#else
    sched_yield();
#endif
}

void Exs_Platform::Exs_YieldProcessor() {
#if defined(EXS_PLATFORM_WINDOWS)
    _mm_pause();
#elif defined(EXS_ARCH_X86) || defined(EXS_ARCH_X64)
    __asm__ __volatile__("pause");
#elif defined(EXS_ARCH_ARM64)
    __asm__ __volatile__("yield");
#else
    // No operation for other architectures
#endif
}

// Memory allocation
void* Exs_Platform::Exs_AllocateAligned(Exs_size_t size, Exs_size_t alignment) {
    EXS_ASSERT_MSG(size > 0, "Allocation size must be greater than 0");
    EXS_ASSERT_MSG((alignment & (alignment - 1)) == 0, "Alignment must be power of two");
    EXS_ASSERT_MSG(alignment >= sizeof(void*), "Alignment must be at least pointer size");
    
    if (size == 0) {
        return nullptr;
    }
    
#if defined(EXS_PLATFORM_WINDOWS)
    return _aligned_malloc(size, alignment);
#elif defined(EXS_PLATFORM_LINUX)
    return memalign(alignment, size);
#elif defined(EXS_PLATFORM_APPLE)
    void* ptr = nullptr;
    posix_memalign(&ptr, alignment, size);
    return ptr;
#else
    // Fallback to over-allocating and aligning manually
    void* original = malloc(size + alignment - 1 + sizeof(void*));
    if (!original) return nullptr;
    
    void* aligned = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(original) + sizeof(void*) + alignment - 1) & ~(alignment - 1));
    *(reinterpret_cast<void**>(aligned) - 1) = original;
    return aligned;
#endif
}

void Exs_Platform::Exs_FreeAligned(void* ptr) {
    if (!ptr) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    _aligned_free(ptr);
#elif defined(EXS_PLATFORM_LINUX) || defined(EXS_PLATFORM_APPLE)
    free(ptr);
#else
    // For manual alignment fallback
    void* original = *(reinterpret_cast<void**>(ptr) - 1);
    free(original);
#endif
}

void* Exs_Platform::Exs_ReallocateAligned(void* ptr, Exs_size_t newSize, Exs_size_t alignment) {
    if (!ptr) {
        return Exs_AllocateAligned(newSize, alignment);
    }
    
    if (newSize == 0) {
        Exs_FreeAligned(ptr);
        return nullptr;
    }
    
    // Get current allocation info
    Exs_size_t oldSize = 0;
    
#if defined(EXS_PLATFORM_WINDOWS)
    oldSize = _aligned_msize(ptr, alignment, 0);
#elif defined(EXS_PLATFORM_LINUX) && defined(_ISOC11_SOURCE)
    oldSize = malloc_usable_size(ptr);
#endif
    
    if (oldSize == 0) {
        // Can't determine old size, allocate new block
        void* newPtr = Exs_AllocateAligned(newSize, alignment);
        if (!newPtr) return nullptr;
        
        // We don't know how much to copy, use conservative estimate
        Exs_FreeAligned(ptr);
        return newPtr;
    }
    
    if (oldSize >= newSize) {
        // Old block is large enough, possibly in-place
        return ptr;
    }
    
    // Need to reallocate
    void* newPtr = Exs_AllocateAligned(newSize, alignment);
    if (!newPtr) return nullptr;
    
    // Copy old data
    memcpy(newPtr, ptr, oldSize);
    Exs_FreeAligned(ptr);
    
    return newPtr;
}

// Memory pages
void* Exs_Platform::Exs_AllocatePage(Exs_size_t size) {
    Exs_size_t pageSize = Exs_GetPageSize();
    Exs_size_t alignedSize = (size + pageSize - 1) & ~(pageSize - 1);
    
#if defined(EXS_PLATFORM_WINDOWS)
    return VirtualAlloc(nullptr, alignedSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    void* ptr = mmap(nullptr, alignedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return nullptr;
    }
    return ptr;
#endif
}

void Exs_Platform::Exs_FreePage(void* ptr, Exs_size_t size) {
    if (!ptr) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, size);
#endif
}

bool Exs_Platform::Exs_ProtectPage(void* ptr, Exs_size_t size, bool read, bool write, bool execute) {
#if defined(EXS_PLATFORM_WINDOWS)
    DWORD protect = 0;
    if (!read && !write && !execute) {
        protect = PAGE_NOACCESS;
    } else if (read && !write && !execute) {
        protect = PAGE_READONLY;
    } else if (read && write && !execute) {
        protect = PAGE_READWRITE;
    } else if (read && !write && execute) {
        protect = PAGE_EXECUTE_READ;
    } else if (read && write && execute) {
        protect = PAGE_EXECUTE_READWRITE;
    } else if (!read && !write && execute) {
        protect = PAGE_EXECUTE;
    } else {
        EXS_ASSERT_MSG(false, "Invalid page protection combination");
        return false;
    }
    
    DWORD oldProtect;
    return VirtualProtect(ptr, size, protect, &oldProtect) != 0;
#else
    int prot = 0;
    if (read) prot |= PROT_READ;
    if (write) prot |= PROT_WRITE;
    if (execute) prot |= PROT_EXEC;
    
    return mprotect(ptr, size, prot) == 0;
#endif
}

Exs_size_t Exs_Platform::Exs_GetPageSize() {
    static Exs_size_t pageSize = 0;
    
    if (pageSize == 0) {
#if defined(EXS_PLATFORM_WINDOWS)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        pageSize = sysInfo.dwPageSize;
#else
        pageSize = sysconf(_SC_PAGESIZE);
#endif
    }
    
    return pageSize;
}

Exs_size_t Exs_Platform::Exs_GetAllocationGranularity() {
    static Exs_size_t granularity = 0;
    
    if (granularity == 0) {
#if defined(EXS_PLATFORM_WINDOWS)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        granularity = sysInfo.dwAllocationGranularity;
#else
        granularity = Exs_GetPageSize();
#endif
    }
    
    return granularity;
}

// Memory barriers
void Exs_Platform::Exs_MemoryBarrier() {
#if defined(EXS_PLATFORM_WINDOWS)
    MemoryBarrier();
#elif defined(EXS_COMPILER_MSVC)
    _ReadWriteBarrier();
    _mm_mfence();
#elif defined(EXS_ARCH_X86) || defined(EXS_ARCH_X64)
    __asm__ __volatile__("mfence" ::: "memory");
#elif defined(EXS_ARCH_ARM64)
    __asm__ __volatile__("dmb ish" ::: "memory");
#else
    __sync_synchronize();
#endif
}

void Exs_Platform::Exs_ReadBarrier() {
#if defined(EXS_PLATFORM_WINDOWS)
    _ReadBarrier();
#elif defined(EXS_ARCH_X86) || defined(EXS_ARCH_X64)
    __asm__ __volatile__("lfence" ::: "memory");
#elif defined(EXS_ARCH_ARM64)
    __asm__ __volatile__("dmb ishld" ::: "memory");
#else
    __sync_synchronize();
#endif
}

void Exs_Platform::Exs_WriteBarrier() {
#if defined(EXS_PLATFORM_WINDOWS)
    _WriteBarrier();
#elif defined(EXS_ARCH_X86) || defined(EXS_ARCH_X64)
    __asm__ __volatile__("sfence" ::: "memory");
#elif defined(EXS_ARCH_ARM64)
    __asm__ __volatile__("dmb ishst" ::: "memory");
#else
    __sync_synchronize();
#endif
}

void Exs_Platform::Exs_CompilerBarrier() {
#if defined(EXS_COMPILER_MSVC)
    _ReadWriteBarrier();
#else
    __asm__ __volatile__("" ::: "memory");
#endif
}

// Atomic operations (implementasi untuk Windows dan GCC/Clang)
Exs_int32 Exs_Platform::Exs_AtomicIncrement(volatile Exs_int32* value) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedIncrement(reinterpret_cast<volatile LONG*>(value));
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_add_and_fetch(value, 1);
#else
    Exs_CompilerBarrier();
    Exs_int32 oldValue = *value;
    *value = oldValue + 1;
    Exs_CompilerBarrier();
    return oldValue + 1;
#endif
}

Exs_int64 Exs_Platform::Exs_AtomicIncrement64(volatile Exs_int64* value) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedIncrement64(value);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_add_and_fetch(value, 1);
#else
    Exs_CompilerBarrier();
    Exs_int64 oldValue = *value;
    *value = oldValue + 1;
    Exs_CompilerBarrier();
    return oldValue + 1;
#endif
}

Exs_int32 Exs_Platform::Exs_AtomicDecrement(volatile Exs_int32* value) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedDecrement(reinterpret_cast<volatile LONG*>(value));
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_sub_and_fetch(value, 1);
#else
    Exs_CompilerBarrier();
    Exs_int32 oldValue = *value;
    *value = oldValue - 1;
    Exs_CompilerBarrier();
    return oldValue - 1;
#endif
}

Exs_int64 Exs_Platform::Exs_AtomicDecrement64(volatile Exs_int64* value) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedDecrement64(value);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_sub_and_fetch(value, 1);
#else
    Exs_CompilerBarrier();
    Exs_int64 oldValue = *value;
    *value = oldValue - 1;
    Exs_CompilerBarrier();
    return oldValue - 1;
#endif
}

Exs_int32 Exs_Platform::Exs_AtomicAdd(volatile Exs_int32* value, Exs_int32 add) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedAdd(reinterpret_cast<volatile LONG*>(value), add);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_add_and_fetch(value, add);
#else
    Exs_CompilerBarrier();
    Exs_int32 oldValue = *value;
    *value = oldValue + add;
    Exs_CompilerBarrier();
    return oldValue + add;
#endif
}

Exs_int64 Exs_Platform::Exs_AtomicAdd64(volatile Exs_int64* value, Exs_int64 add) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedAdd64(value, add);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_add_and_fetch(value, add);
#else
    Exs_CompilerBarrier();
    Exs_int64 oldValue = *value;
    *value = oldValue + add;
    Exs_CompilerBarrier();
    return oldValue + add;
#endif
}

Exs_int32 Exs_Platform::Exs_AtomicExchange(volatile Exs_int32* value, Exs_int32 exchange) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedExchange(reinterpret_cast<volatile LONG*>(value), exchange);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_lock_test_and_set(value, exchange);
#else
    Exs_CompilerBarrier();
    Exs_int32 oldValue = *value;
    *value = exchange;
    Exs_CompilerBarrier();
    return oldValue;
#endif
}

Exs_int64 Exs_Platform::Exs_AtomicExchange64(volatile Exs_int64* value, Exs_int64 exchange) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedExchange64(value, exchange);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_lock_test_and_set(value, exchange);
#else
    Exs_CompilerBarrier();
    Exs_int64 oldValue = *value;
    *value = exchange;
    Exs_CompilerBarrier();
    return oldValue;
#endif
}

Exs_int32 Exs_Platform::Exs_AtomicCompareExchange(volatile Exs_int32* dest, Exs_int32 exchange, Exs_int32 comparand) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(dest), exchange, comparand);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_val_compare_and_swap(dest, comparand, exchange);
#else
    Exs_CompilerBarrier();
    Exs_int32 oldValue = *dest;
    if (oldValue == comparand) {
        *dest = exchange;
    }
    Exs_CompilerBarrier();
    return oldValue;
#endif
}

Exs_int64 Exs_Platform::Exs_AtomicCompareExchange64(volatile Exs_int64* dest, Exs_int64 exchange, Exs_int64 comparand) {
#if defined(EXS_PLATFORM_WINDOWS)
    return InterlockedCompareExchange64(dest, exchange, comparand);
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    return __sync_val_compare_and_swap(dest, comparand, exchange);
#else
    Exs_CompilerBarrier();
    Exs_int64 oldValue = *dest;
    if (oldValue == comparand) {
        *dest = exchange;
    }
    Exs_CompilerBarrier();
    return oldValue;
#endif
}

// Thread local storage
Exs_uint32 Exs_Platform::Exs_TLSAlloc() {
#if defined(EXS_PLATFORM_WINDOWS)
    return TlsAlloc();
#else
    pthread_key_t key;
    if (pthread_key_create(&key, nullptr) != 0) {
        return EXS_TLS_OUT_OF_INDEXES;
    }
    return static_cast<Exs_uint32>(key);
#endif
}

bool Exs_Platform::Exs_TLSFree(Exs_uint32 index) {
#if defined(EXS_PLATFORM_WINDOWS)
    return TlsFree(index) != 0;
#else
    return pthread_key_delete(static_cast<pthread_key_t>(index)) == 0;
#endif
}

void* Exs_Platform::Exs_TLSGetValue(Exs_uint32 index) {
#if defined(EXS_PLATFORM_WINDOWS)
    return TlsGetValue(index);
#else
    return pthread_getspecific(static_cast<pthread_key_t>(index));
#endif
}

bool Exs_Platform::Exs_TLSSetValue(Exs_uint32 index, void* value) {
#if defined(EXS_PLATFORM_WINDOWS)
    return TlsSetValue(index, value) != 0;
#else
    return pthread_setspecific(static_cast<pthread_key_t>(index), value) == 0;
#endif
}

// Thread functions
Exs_uint32 Exs_Platform::Exs_GetCurrentThreadId() {
#if defined(EXS_PLATFORM_WINDOWS)
    return GetCurrentThreadId();
#else
    return static_cast<Exs_uint32>(syscall(SYS_gettid));
#endif
}

void Exs_Platform::Exs_SetThreadName(const char* name) {
#if defined(EXS_PLATFORM_WINDOWS)
    // Windows thread naming
    #pragma pack(push, 8)
    typedef struct tagTHREADNAME_INFO {
        DWORD dwType;      // Must be 0x1000
        LPCSTR szName;     // Pointer to name (in user address space)
        DWORD dwThreadID;  // Thread ID (-1 = caller thread)
        DWORD dwFlags;     // Reserved for future use, must be zero
    } THREADNAME_INFO;
    #pragma pack(pop)
    
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;
    
    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
#elif defined(EXS_PLATFORM_LINUX) || defined(EXS_PLATFORM_APPLE)
    pthread_setname_np(pthread_self(), name);
#endif
}

void Exs_Platform::Exs_SetThreadPriority(Exs_int32 priority) {
#if defined(EXS_PLATFORM_WINDOWS)
    int winPriority;
    switch (priority) {
        case -2: winPriority = THREAD_PRIORITY_LOWEST; break;
        case -1: winPriority = THREAD_PRIORITY_BELOW_NORMAL; break;
        case 0:  winPriority = THREAD_PRIORITY_NORMAL; break;
        case 1:  winPriority = THREAD_PRIORITY_ABOVE_NORMAL; break;
        case 2:  winPriority = THREAD_PRIORITY_HIGHEST; break;
        case 3:  winPriority = THREAD_PRIORITY_TIME_CRITICAL; break;
        default: winPriority = THREAD_PRIORITY_NORMAL; break;
    }
    SetThreadPriority(GetCurrentThread(), winPriority);
#else
    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    
    // Map priority to nice value or sched_priority
    param.sched_priority = priority;
    pthread_setschedparam(pthread_self(), policy, &param);
#endif
}

void Exs_Platform::Exs_SetThreadAffinityMask(Exs_uint64 mask) {
#if defined(EXS_PLATFORM_WINDOWS)
    SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(mask));
#elif defined(EXS_PLATFORM_LINUX) || defined(EXS_PLATFORM_APPLE)
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    
    for (Exs_uint32 i = 0; i < sizeof(mask) * 8; ++i) {
        if (mask & (1ULL << i)) {
            CPU_SET(i, &cpuSet);
        }
    }
    
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet);
#endif
}

// Process functions
Exs_uint32 Exs_Platform::Exs_GetCurrentProcessId() {
#if defined(EXS_PLATFORM_WINDOWS)
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

Exs_size_t Exs_Platform::Exs_GetProcessMemoryUsage() {
#if defined(EXS_PLATFORM_WINDOWS)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#elif defined(EXS_PLATFORM_LINUX)
    // Read from /proc/self/statm
    FILE* file = fopen("/proc/self/statm", "r");
    if (file) {
        unsigned long size, resident, share, text, lib, data, dt;
        fscanf(file, "%lu %lu %lu %lu %lu %lu %lu", &size, &resident, &share, &text, &lib, &data, &dt);
        fclose(file);
        return resident * sysconf(_SC_PAGESIZE);
    }
    return 0;
#elif defined(EXS_PLATFORM_APPLE)
    struct task_basic_info info;
    mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS) {
        return info.resident_size;
    }
    return 0;
#else
    return 0;
#endif
}

Exs_size_t Exs_Platform::Exs_GetProcessPeakMemoryUsage() {
#if defined(EXS_PLATFORM_WINDOWS)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.PeakWorkingSetSize;
    }
    return 0;
#elif defined(EXS_PLATFORM_LINUX)
    // Read from /proc/self/status
    FILE* file = fopen("/proc/self/status", "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "VmPeak:", 7) == 0) {
                Exs_size_t value;
                sscanf(line + 7, "%lu", &value);
                fclose(file);
                return value * 1024; // Convert from KB to bytes
            }
        }
        fclose(file);
    }
    return 0;
#else
    return Exs_GetProcessMemoryUsage();
#endif
}

void Exs_Platform::Exs_SetProcessPriority(Exs_int32 priority) {
#if defined(EXS_PLATFORM_WINDOWS)
    int winPriority;
    switch (priority) {
        case -2: winPriority = IDLE_PRIORITY_CLASS; break;
        case -1: winPriority = BELOW_NORMAL_PRIORITY_CLASS; break;
        case 0:  winPriority = NORMAL_PRIORITY_CLASS; break;
        case 1:  winPriority = ABOVE_NORMAL_PRIORITY_CLASS; break;
        case 2:  winPriority = HIGH_PRIORITY_CLASS; break;
        case 3:  winPriority = REALTIME_PRIORITY_CLASS; break;
        default: winPriority = NORMAL_PRIORITY_CLASS; break;
    }
    SetPriorityClass(GetCurrentProcess(), winPriority);
#else
    // On Unix, use nice value
    nice(priority);
#endif
}

// Module handling
void* Exs_Platform::Exs_LoadLibrary(const char* filename) {
#if defined(EXS_PLATFORM_WINDOWS)
    return LoadLibraryA(filename);
#else
    return dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
#endif
}

bool Exs_Platform::Exs_FreeLibrary(void* handle) {
#if defined(EXS_PLATFORM_WINDOWS)
    return FreeLibrary(static_cast<HMODULE>(handle)) != 0;
#else
    return dlclose(handle) == 0;
#endif
}

void* Exs_Platform::Exs_GetProcAddress(void* handle, const char* procname) {
#if defined(EXS_PLATFORM_WINDOWS)
    return GetProcAddress(static_cast<HMODULE>(handle), procname);
#else
    return dlsym(handle, procname);
#endif
}

// File system functions (implementasi parsial karena panjang)
bool Exs_Platform::Exs_CreateDirectory(const char* path) {
#if defined(EXS_PLATFORM_WINDOWS)
    return CreateDirectoryA(path, nullptr) != 0;
#else
    return mkdir(path, 0755) == 0;
#endif
}

bool Exs_Platform::Exs_DeleteDirectory(const char* path) {
#if defined(EXS_PLATFORM_WINDOWS)
    return RemoveDirectoryA(path) != 0;
#else
    return rmdir(path) == 0;
#endif
}

bool Exs_Platform::Exs_CreateFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

bool Exs_Platform::Exs_DeleteFile(const char* filename) {
#if defined(EXS_PLATFORM_WINDOWS)
    return DeleteFileA(filename) != 0;
#else
    return remove(filename) == 0;
#endif
}

bool Exs_Platform::Exs_FileExists(const char* filename) {
#if defined(EXS_PLATFORM_WINDOWS)
    DWORD attrib = GetFileAttributesA(filename);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return stat(filename, &st) == 0 && S_ISREG(st.st_mode);
#endif
}

bool Exs_Platform::Exs_DirectoryExists(const char* path) {
#if defined(EXS_PLATFORM_WINDOWS)
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

Exs_uint64 Exs_Platform::Exs_GetFileSize(const char* filename) {
#if defined(EXS_PLATFORM_WINDOWS)
    WIN32_FILE_ATTRIBUTE_DATA fileAttr;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &fileAttr)) {
        return (static_cast<Exs_uint64>(fileAttr.nFileSizeHigh) << 32) | fileAttr.nFileSizeLow;
    }
    return 0;
#else
    struct stat st;
    if (stat(filename, &st) == 0) {
        return static_cast<Exs_uint64>(st.st_size);
    }
    return 0;
#endif
}

// ... (lanjutan implementasi file system lainnya)

// Path utilities
char Exs_Platform::Exs_GetPathSeparator() {
#if defined(EXS_PLATFORM_WINDOWS)
    return '\\';
#else
    return '/';
#endif
}

void Exs_Platform::Exs_NormalizePath(char* path) {
    if (!path) return;
    
    char separator = Exs_GetPathSeparator();
    char otherSeparator = (separator == '/') ? '\\' : '/';
    
    // Replace other separator with platform separator
    for (char* p = path; *p; ++p) {
        if (*p == otherSeparator) {
            *p = separator;
        }
    }
    
    // Remove duplicate separators
    char* write = path;
    char* read = path;
    bool lastWasSeparator = false;
    
    while (*read) {
        if (*read == separator) {
            if (!lastWasSeparator) {
                *write++ = separator;
                lastWasSeparator = true;
            }
        } else {
            *write++ = *read;
            lastWasSeparator = false;
        }
        ++read;
    }
    *write = '\0';
    
    // Remove trailing separator
    Exs_size_t len = strlen(path);
    if (len > 1 && path[len - 1] == separator) {
        path[len - 1] = '\0';
    }
}

void Exs_Platform::Exs_GetAbsolutePath(const char* relativePath, char* absolutePath, Exs_size_t bufferSize) {
#if defined(EXS_PLATFORM_WINDOWS)
    GetFullPathNameA(relativePath, static_cast<DWORD>(bufferSize), absolutePath, nullptr);
#else
    realpath(relativePath, absolutePath);
#endif
}

void Exs_Platform::Exs_GetCurrentDirectory(char* buffer, Exs_size_t bufferSize) {
#if defined(EXS_PLATFORM_WINDOWS)
    GetCurrentDirectoryA(static_cast<DWORD>(bufferSize), buffer);
#else
    getcwd(buffer, bufferSize);
#endif
}

bool Exs_Platform::Exs_SetCurrentDirectory(const char* path) {
#if defined(EXS_PLATFORM_WINDOWS)
    return SetCurrentDirectoryA(path) != 0;
#else
    return chdir(path) == 0;
#endif
}

void Exs_Platform::Exs_GetExecutablePath(char* buffer, Exs_size_t bufferSize) {
#if defined(EXS_PLATFORM_WINDOWS)
    GetModuleFileNameA(nullptr, buffer, static_cast<DWORD>(bufferSize));
#elif defined(EXS_PLATFORM_LINUX)
    readlink("/proc/self/exe", buffer, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
#elif defined(EXS_PLATFORM_APPLE)
    uint32_t size = static_cast<uint32_t>(bufferSize);
    _NSGetExecutablePath(buffer, &size);
#endif
}

// Debugging
void Exs_Platform::Exs_DebugBreak() {
#if defined(EXS_PLATFORM_WINDOWS)
    __debugbreak();
#elif defined(EXS_COMPILER_GCC) || defined(EXS_COMPILER_CLANG)
    __builtin_trap();
#else
    raise(SIGTRAP);
#endif
}

void Exs_Platform::Exs_OutputDebugString(const char* message) {
#if defined(EXS_PLATFORM_WINDOWS)
    OutputDebugStringA(message);
#elif defined(EXS_PLATFORM_LINUX) || defined(EXS_PLATFORM_APPLE)
    // Write to stderr for Linux/macOS
    fprintf(stderr, "%s", message);
#endif
}

bool Exs_Platform::Exs_IsDebuggerPresent() {
#if defined(EXS_PLATFORM_WINDOWS)
    return IsDebuggerPresent() != 0;
#elif defined(EXS_PLATFORM_LINUX)
    // Check /proc/self/status for TracerPid
    FILE* file = fopen("/proc/self/status", "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "TracerPid:", 10) == 0) {
                int pid;
                sscanf(line + 10, "%d", &pid);
                fclose(file);
                return pid != 0;
            }
        }
        fclose(file);
    }
    return false;
#elif defined(EXS_PLATFORM_APPLE)
    // Check for being traced
    int mib[4];
    struct kinfo_proc info;
    size_t size = sizeof(info);
    
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    
    if (sysctl(mib, 4, &info, &size, nullptr, 0) == 0) {
        return (info.kp_proc.p_flag & P_TRACED) != 0;
    }
    return false;
#else
    return false;
#endif
}

void Exs_Platform::Exs_BreakIntoDebugger() {
    if (Exs_IsDebuggerPresent()) {
        Exs_DebugBreak();
    }
}

// Error handling
Exs_uint32 Exs_Platform::Exs_GetLastError() {
#if defined(EXS_PLATFORM_WINDOWS)
    return GetLastError();
#else
    return errno;
#endif
}

void Exs_Platform::Exs_SetLastError(Exs_uint32 error) {
#if defined(EXS_PLATFORM_WINDOWS)
    SetLastError(error);
#else
    errno = error;
#endif
}

const char* Exs_Platform::Exs_GetErrorString(Exs_uint32 error) {
    static thread_local char buffer[256];
    
#if defined(EXS_PLATFORM_WINDOWS)
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  buffer, sizeof(buffer), nullptr);
#else
    strerror_r(error, buffer, sizeof(buffer));
#endif
    
    return buffer;
}

// System information
const Exs_SystemInfo& Exs_Platform::Exs_GetSystemInfo() {
    return Exs_SystemInfo::Exs_GetInstance();
}

const Exs_CPUInfo& Exs_Platform::Exs_GetCPUInfo() {
    return Exs_CPUInfo::Exs_GetInstance();
}

const Exs_MemoryInfo& Exs_Platform::Exs_GetMemoryInfo() {
    return Exs_MemoryInfo::Exs_GetInstance();
}

// Platform-specific features
bool Exs_Platform::Exs_IsElevated() {
#if defined(EXS_PLATFORM_WINDOWS)
    BOOL isElevated = FALSE;
    HANDLE token = nullptr;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(TOKEN_ELEVATION);
        
        if (GetTokenInformation(token, TokenElevation, &elevation, size, &size)) {
            isElevated = elevation.TokenIsElevated;
        }
        
        CloseHandle(token);
    }
    
    return isElevated != 0;
#else
    return geteuid() == 0;
#endif
}

// ... (lanjutan dari kode sebelumnya)

// Platform-specific features lanjutan
bool Exs_Platform::Exs_IsService() {
#if defined(EXS_PLATFORM_WINDOWS)
    // Check if running as a service
    typedef BOOL (WINAPI *LPFN_ISSERVICE)();
    HMODULE hAdvApi32 = LoadLibraryA("advapi32.dll");
    if (hAdvApi32) {
        LPFN_ISSERVICE pIsService = (LPFN_ISSERVICE)GetProcAddress(hAdvApi32, "IsService");
        if (pIsService) {
            BOOL result = pIsService();
            FreeLibrary(hAdvApi32);
            return result != 0;
        }
        FreeLibrary(hAdvApi32);
    }
#endif
    return false;
}

bool Exs_Platform::Exs_IsUserAdmin() {
#if defined(EXS_PLATFORM_WINDOWS)
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        BOOL isMember = FALSE;
        if (!CheckTokenMembership(nullptr, AdministratorsGroup, &isMember)) {
            isMember = FALSE;
        }
        FreeSid(AdministratorsGroup);
        return isMember != 0;
    }
#endif
    return Exs_IsElevated();
}

// Power management
bool Exs_Platform::Exs_IsOnBatteryPower() {
#if defined(EXS_PLATFORM_WINDOWS)
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        return status.ACLineStatus == 0; // 0 = Offline, 1 = Online
    }
#elif defined(EXS_PLATFORM_LINUX)
    // Check for AC power in Linux
    FILE* file = fopen("/sys/class/power_supply/AC/online", "r");
    if (file) {
        char line[16];
        if (fgets(line, sizeof(line), file)) {
            fclose(file);
            return atoi(line) == 0;
        }
        fclose(file);
    }
#elif defined(EXS_PLATFORM_APPLE)
    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
    
    if (sources) {
        CFIndex count = CFArrayGetCount(sources);
        for (CFIndex i = 0; i < count; i++) {
            CFTypeRef source = CFArrayGetValueAtIndex(sources, i);
            CFDictionaryRef info = IOPSGetPowerSourceDescription(blob, source);
            
            if (info) {
                CFStringRef powerSource = (CFStringRef)CFDictionaryGetValue(info, CFSTR(kIOPSPowerSourceStateKey));
                if (powerSource && CFStringCompare(powerSource, CFSTR(kIOPSBatteryPowerValue), 0) == kCFCompareEqualTo) {
                    CFRelease(sources);
                    CFRelease(blob);
                    return true;
                }
            }
        }
        CFRelease(sources);
    }
    if (blob) CFRelease(blob);
#endif
    return false;
}

Exs_uint32 Exs_Platform::Exs_GetBatteryLifePercent() {
#if defined(EXS_PLATFORM_WINDOWS)
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        return status.BatteryLifePercent;
    }
#elif defined(EXS_PLATFORM_LINUX)
    // Read from /sys/class/power_supply/BAT0/capacity
    FILE* file = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (file) {
        char line[16];
        if (fgets(line, sizeof(line), file)) {
            fclose(file);
            return static_cast<Exs_uint32>(atoi(line));
        }
        fclose(file);
    }
#elif defined(EXS_PLATFORM_APPLE)
    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
    
    if (sources) {
        CFIndex count = CFArrayGetCount(sources);
        for (CFIndex i = 0; i < count; i++) {
            CFTypeRef source = CFArrayGetValueAtIndex(sources, i);
            CFDictionaryRef info = IOPSGetPowerSourceDescription(blob, source);
            
            if (info) {
                CFNumberRef capacity = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kIOPSCurrentCapacityKey));
                if (capacity) {
                    Exs_uint32 percent;
                    CFNumberGetValue(capacity, kCFNumberSInt32Type, &percent);
                    CFRelease(sources);
                    CFRelease(blob);
                    return percent;
                }
            }
        }
        CFRelease(sources);
    }
    if (blob) CFRelease(blob);
#endif
    return 100; // Default to 100% if unknown
}

bool Exs_Platform::Exs_GetPowerStatus(bool* acOnline, bool* batteryCharging, 
                                     Exs_uint32* batteryLifePercent, Exs_uint32* batteryLifeTime) {
    if (acOnline) *acOnline = false;
    if (batteryCharging) *batteryCharging = false;
    if (batteryLifePercent) *batteryLifePercent = 0;
    if (batteryLifeTime) *batteryLifeTime = 0;
    
#if defined(EXS_PLATFORM_WINDOWS)
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        if (acOnline) *acOnline = (status.ACLineStatus == 1);
        if (batteryCharging) *batteryCharging = (status.BatteryFlag & 8) != 0; // 8 = Charging
        if (batteryLifePercent) *batteryLifePercent = status.BatteryLifePercent;
        if (batteryLifeTime) *batteryLifeTime = status.BatteryLifeTime;
        return true;
    }
#elif defined(EXS_PLATFORM_LINUX)
    // Read multiple sysfs files
    if (acOnline) {
        FILE* file = fopen("/sys/class/power_supply/AC/online", "r");
        if (file) {
            char line[16];
            if (fgets(line, sizeof(line), file)) {
                *acOnline = atoi(line) != 0;
            }
            fclose(file);
        }
    }
    
    if (batteryCharging) {
        FILE* file = fopen("/sys/class/power_supply/BAT0/status", "r");
        if (file) {
            char line[16];
            if (fgets(line, sizeof(line), file)) {
                *batteryCharging = (strncmp(line, "Charging", 8) == 0);
            }
            fclose(file);
        }
    }
    
    if (batteryLifePercent) {
        *batteryLifePercent = Exs_GetBatteryLifePercent();
    }
    
    return true;
#endif
    return false;
}

// Clipboard (Windows implementation only)
bool Exs_Platform::Exs_GetClipboardText(char* buffer, Exs_size_t bufferSize) {
#if defined(EXS_PLATFORM_WINDOWS)
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        CloseClipboard();
        return false;
    }
    
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        CloseClipboard();
        return false;
    }
    
    strncpy_s(buffer, bufferSize, pszText, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
    
    GlobalUnlock(hData);
    CloseClipboard();
    return true;
#else
    // Linux/macOS clipboard access requires X11 or Cocoa APIs
    // For simplicity, return false on non-Windows
    EXS_UNUSED(buffer);
    EXS_UNUSED(bufferSize);
    return false;
#endif
}

bool Exs_Platform::Exs_SetClipboardText(const char* text) {
#if defined(EXS_PLATFORM_WINDOWS)
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    EmptyClipboard();
    
    Exs_size_t len = strlen(text) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem == nullptr) {
        CloseClipboard();
        return false;
    }
    
    char* pMem = static_cast<char*>(GlobalLock(hMem));
    if (pMem == nullptr) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    
    strcpy_s(pMem, len, text);
    GlobalUnlock(hMem);
    
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
    return true;
#else
    EXS_UNUSED(text);
    return false;
#endif
}

// UUID generation
void Exs_Platform::Exs_GenerateUUID(char* buffer, Exs_size_t bufferSize) {
#if defined(EXS_PLATFORM_WINDOWS)
    GUID guid;
    if (CoCreateGuid(&guid) == S_OK) {
        snprintf(buffer, bufferSize,
                "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    } else {
        buffer[0] = '\0';
    }
#elif defined(EXS_PLATFORM_LINUX)
    FILE* file = fopen("/proc/sys/kernel/random/uuid", "r");
    if (file) {
        fgets(buffer, bufferSize, file);
        fclose(file);
        // Remove newline if present
        char* newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
    } else {
        buffer[0] = '\0';
    }
#else
    // Simple random UUID generation as fallback
    srand(static_cast<unsigned int>(Exs_GetPerformanceCounter()));
    snprintf(buffer, bufferSize,
            "%08X-%04X-%04X-%04X-%08X%04X",
            rand() & 0xFFFFFFFF,
            rand() & 0xFFFF,
            (rand() & 0x0FFF) | 0x4000, // Version 4
            (rand() & 0x3FFF) | 0x8000, // Variant RFC4122
            rand() & 0xFFFFFFFF,
            rand() & 0xFFFF);
#endif
}

// Random number generation
bool Exs_Platform::Exs_GenerateRandomBytes(void* buffer, Exs_size_t size) {
#if defined(EXS_PLATFORM_WINDOWS)
    HCRYPTPROV hProv;
    if (CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        BOOL success = CryptGenRandom(hProv, static_cast<DWORD>(size), static_cast<BYTE*>(buffer));
        CryptReleaseContext(hProv, 0);
        return success != FALSE;
    }
    return false;
#elif defined(EXS_PLATFORM_LINUX)
    FILE* file = fopen("/dev/urandom", "rb");
    if (file) {
        Exs_size_t bytesRead = fread(buffer, 1, size, file);
        fclose(file);
        return bytesRead == size;
    }
    return false;
#elif defined(EXS_PLATFORM_APPLE)
    // Use arc4random_buf on macOS/iOS
    arc4random_buf(buffer, size);
    return true;
#else
    // Fallback to less secure random
    srand(static_cast<unsigned int>(Exs_GetPerformanceCounter()));
    for (Exs_size_t i = 0; i < size; ++i) {
        static_cast<unsigned char*>(buffer)[i] = rand() & 0xFF;
    }
    return true;
#endif
}

// Command line
Exs_int32 Exs_Platform::Exs_GetCommandLineArgCount() {
#if defined(EXS_PLATFORM_WINDOWS)
    return __argc;
#else
    extern char** environ;
    extern char** __argv;
    extern int __argc;
    return __argc;
#endif
}

const char* Exs_Platform::Exs_GetCommandLineArg(Exs_int32 index) {
#if defined(EXS_PLATFORM_WINDOWS)
    if (index >= 0 && index < __argc) {
        return __argv[index];
    }
#else
    extern char** __argv;
    extern int __argc;
    if (index >= 0 && index < __argc) {
        return __argv[index];
    }
#endif
    return nullptr;
}

const char* Exs_Platform::Exs_GetCommandLine() {
#if defined(EXS_PLATFORM_WINDOWS)
    return GetCommandLineA();
#else
    extern char** environ;
    // Not directly available, reconstruct from arguments
    static thread_local char commandLine[4096] = {0};
    commandLine[0] = '\0';
    
    Exs_int32 argc = Exs_GetCommandLineArgCount();
    for (Exs_int32 i = 0; i < argc; ++i) {
        const char* arg = Exs_GetCommandLineArg(i);
        if (arg) {
            strcat(commandLine, arg);
            if (i < argc - 1) {
                strcat(commandLine, " ");
            }
        }
    }
    return commandLine;
#endif
}

// Application exit
void Exs_Platform::Exs_ExitProcess(Exs_int32 exitCode) {
#if defined(EXS_PLATFORM_WINDOWS)
    ExitProcess(exitCode);
#else
    _exit(exitCode);
#endif
}

void Exs_Platform::Exs_AbortProcess() {
    abort();
}

void Exs_Platform::Exs_TerminateProcess(Exs_uint32 processId, Exs_int32 exitCode) {
#if defined(EXS_PLATFORM_WINDOWS)
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess) {
        TerminateProcess(hProcess, exitCode);
        CloseHandle(hProcess);
    }
#else
    kill(processId, SIGKILL);
#endif
}

// Dynamic code generation
void* Exs_Platform::Exs_AllocateExecutableMemory(Exs_size_t size) {
#if defined(EXS_PLATFORM_WINDOWS)
    return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC, 
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (ptr == MAP_FAILED) ? nullptr : ptr;
#endif
}

void Exs_Platform::Exs_FreeExecutableMemory(void* ptr, Exs_size_t size) {
    if (!ptr) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, size);
#endif
}

bool Exs_Platform::Exs_FlushInstructionCache(void* ptr, Exs_size_t size) {
#if defined(EXS_PLATFORM_WINDOWS)
    return FlushInstructionCache(GetCurrentProcess(), ptr, size) != 0;
#elif defined(EXS_PLATFORM_LINUX) || defined(EXS_PLATFORM_APPLE)
    // On POSIX systems, mprotect with PROT_EXEC should handle cache coherency
    // For ARM, we might need __clear_cache
    #if defined(__GNUC__) && (defined(__arm__) || defined(__aarch64__))
        __builtin___clear_cache(ptr, (char*)ptr + size);
    #endif
    return true;
#else
    EXS_UNUSED(ptr);
    EXS_UNUSED(size);
    return true;
#endif
}

// Signal/exception handling
bool Exs_Platform::Exs_SetSignalHandler(Exs_int32 signal, void (*handler)(Exs_int32)) {
#if defined(EXS_PLATFORM_WINDOWS)
    // Windows uses structured exception handling, not POSIX signals
    return false;
#else
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    return sigaction(signal, &sa, nullptr) == 0;
#endif
}

bool Exs_Platform::Exs_SetUnhandledExceptionFilter(void (*filter)(void*)) {
#if defined(EXS_PLATFORM_WINDOWS)
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)filter);
    return true;
#else
    // On Unix, we can use sigaction with SA_SIGINFO for advanced signal handling
    EXS_UNUSED(filter);
    return false;
#endif
}

// Critical sections (implementasi lengkap)
void* Exs_Platform::Exs_CreateCriticalSection() {
#if defined(EXS_PLATFORM_WINDOWS)
    CRITICAL_SECTION* cs = new CRITICAL_SECTION;
    InitializeCriticalSection(cs);
    return cs;
#else
    pthread_mutex_t* mutex = new pthread_mutex_t;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    return mutex;
#endif
}

void Exs_Platform::Exs_DeleteCriticalSection(void* cs) {
    if (!cs) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(cs));
    delete static_cast<CRITICAL_SECTION*>(cs);
#else
    pthread_mutex_destroy(static_cast<pthread_mutex_t*>(cs));
    delete static_cast<pthread_mutex_t*>(cs);
#endif
}

void Exs_Platform::Exs_EnterCriticalSection(void* cs) {
    if (!cs) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    EnterCriticalSection(static_cast<CRITICAL_SECTION*>(cs));
#else
    pthread_mutex_lock(static_cast<pthread_mutex_t*>(cs));
#endif
}

bool Exs_Platform::Exs_TryEnterCriticalSection(void* cs) {
    if (!cs) return false;
    
#if defined(EXS_PLATFORM_WINDOWS)
    return TryEnterCriticalSection(static_cast<CRITICAL_SECTION*>(cs)) != 0;
#else
    return pthread_mutex_trylock(static_cast<pthread_mutex_t*>(cs)) == 0;
#endif
}

void Exs_Platform::Exs_LeaveCriticalSection(void* cs) {
    if (!cs) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(cs));
#else
    pthread_mutex_unlock(static_cast<pthread_mutex_t*>(cs));
#endif
}

// Event objects (Windows hanya)
void* Exs_Platform::Exs_CreateEvent(bool manualReset, bool initialState, const char* name) {
#if defined(EXS_PLATFORM_WINDOWS)
    return CreateEventA(nullptr, manualReset ? TRUE : FALSE, 
                       initialState ? TRUE : FALSE, name);
#else
    // On Unix, we can use pthread condition variables or pipe/socketpair
    // Simplified implementation using condition variable
    struct Exs_Event {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        bool signaled;
        bool manualReset;
    };
    
    Exs_Event* event = new Exs_Event;
    pthread_mutex_init(&event->mutex, nullptr);
    pthread_cond_init(&event->cond, nullptr);
    event->signaled = initialState;
    event->manualReset = manualReset;
    
    return event;
#endif
}

void Exs_Platform::Exs_DestroyEvent(void* event) {
    if (!event) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    CloseHandle(event);
#else
    Exs_Event* ev = static_cast<Exs_Event*>(event);
    pthread_cond_destroy(&ev->cond);
    pthread_mutex_destroy(&ev->mutex);
    delete ev;
#endif
}

bool Exs_Platform::Exs_SetEvent(void* event) {
    if (!event) return false;
    
#if defined(EXS_PLATFORM_WINDOWS)
    return SetEvent(event) != 0;
#else
    Exs_Event* ev = static_cast<Exs_Event*>(event);
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = true;
    if (ev->manualReset) {
        pthread_cond_broadcast(&ev->cond);
    } else {
        pthread_cond_signal(&ev->cond);
    }
    pthread_mutex_unlock(&ev->mutex);
    return true;
#endif
}

bool Exs_Platform::Exs_ResetEvent(void* event) {
    if (!event) return false;
    
#if defined(EXS_PLATFORM_WINDOWS)
    return ResetEvent(event) != 0;
#else
    Exs_Event* ev = static_cast<Exs_Event*>(event);
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = false;
    pthread_mutex_unlock(&ev->mutex);
    return true;
#endif
}

bool Exs_Platform::Exs_PulseEvent(void* event) {
    if (!event) return false;
    
#if defined(EXS_PLATFORM_WINDOWS)
    return PulseEvent(event) != 0;
#else
    Exs_Event* ev = static_cast<Exs_Event*>(event);
    pthread_mutex_lock(&ev->mutex);
    if (ev->manualReset) {
        pthread_cond_broadcast(&ev->cond);
    } else {
        pthread_cond_signal(&ev->cond);
    }
    ev->signaled = false;
    pthread_mutex_unlock(&ev->mutex);
    return true;
#endif
}

Exs_uint32 Exs_Platform::Exs_WaitForSingleObject(void* handle, Exs_uint32 milliseconds) {
#if defined(EXS_PLATFORM_WINDOWS)
    return WaitForSingleObject(handle, milliseconds);
#else
    if (!handle) return 0xFFFFFFFF;
    
    // Check if it's an event
    if (reinterpret_cast<uintptr_t>(handle) & 0x1) {
        // Handle other object types
    } else {
        Exs_Event* ev = static_cast<Exs_Event*>(handle);
        pthread_mutex_lock(&ev->mutex);
        
        if (!ev->signaled) {
            if (milliseconds == 0xFFFFFFFF) { // INFINITE
                while (!ev->signaled) {
                    pthread_cond_wait(&ev->cond, &ev->mutex);
                }
            } else {
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_nsec += (milliseconds % 1000) * 1000000;
                ts.tv_sec += milliseconds / 1000 + ts.tv_nsec / 1000000000;
                ts.tv_nsec %= 1000000000;
                
                int result = pthread_cond_timedwait(&ev->cond, &ev->mutex, &ts);
                if (result == ETIMEDOUT) {
                    pthread_mutex_unlock(&ev->mutex);
                    return 0x00000102; // WAIT_TIMEOUT
                }
            }
        }
        
        if (!ev->manualReset) {
            ev->signaled = false;
        }
        
        pthread_mutex_unlock(&ev->mutex);
        return 0; // WAIT_OBJECT_0
    }
    return 0xFFFFFFFF;
#endif
}

Exs_uint32 Exs_Platform::Exs_WaitForMultipleObjects(Exs_uint32 count, void** handles, 
                                                   bool waitAll, Exs_uint32 milliseconds) {
#if defined(EXS_PLATFORM_WINDOWS)
    return WaitForMultipleObjects(count, handles, waitAll ? TRUE : FALSE, milliseconds);
#else
    // Simplified implementation for Unix
    if (count == 0) return 0xFFFFFFFF;
    
    if (count == 1) {
        return Exs_WaitForSingleObject(handles[0], milliseconds);
    }
    
    // For multiple objects, we need more complex logic
    // This is a simplified implementation that waits on any
    for (Exs_uint32 i = 0; i < count; ++i) {
        Exs_uint32 result = Exs_WaitForSingleObject(handles[i], 0);
        if (result == 0) {
            return i; // WAIT_OBJECT_0 + i
        }
    }
    
    if (milliseconds == 0) {
        return 0x00000102; // WAIT_TIMEOUT
    }
    
    // Need to implement proper waiting for multiple objects
    // For now, just wait on first object
    return Exs_WaitForSingleObject(handles[0], milliseconds);
#endif
}

// ... (implementasi untuk Mutex, Semaphore, Timer, dan Window message processing
// terlalu panjang untuk dimasukkan semua di sini, tapi saya akan berikan kerangka)

// Mutex objects
void* Exs_Platform::Exs_CreateMutex(bool initialOwner, const char* name) {
#if defined(EXS_PLATFORM_WINDOWS)
    return CreateMutexA(nullptr, initialOwner ? TRUE : FALSE, name);
#else
    pthread_mutex_t* mutex = new pthread_mutex_t;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    if (initialOwner) {
        pthread_mutex_lock(mutex);
    }
    
    return mutex;
#endif
}

void Exs_Platform::Exs_DestroyMutex(void* mutex) {
    if (!mutex) return;
    
#if defined(EXS_PLATFORM_WINDOWS)
    CloseHandle(mutex);
#else
    pthread_mutex_destroy(static_cast<pthread_mutex_t*>(mutex));
    delete static_cast<pthread_mutex_t*>(mutex);
#endif
}

bool Exs_Platform::Exs_ReleaseMutex(void* mutex) {
    if (!mutex) return false;
    
#if defined(EXS_PLATFORM_WINDOWS)
    return ReleaseMutex(mutex) != 0;
#else
    return pthread_mutex_unlock(static_cast<pthread_mutex_t*>(mutex)) == 0;
#endif
}

// Window message processing (Windows specific)
#ifdef EXS_PLATFORM_WINDOWS
bool Exs_Platform::Exs_PumpMessage() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

bool Exs_Platform::Exs_PeekMessage(void* msg) {
    return PeekMessage(static_cast<MSG*>(msg), nullptr, 0, 0, PM_REMOVE) != 0;
}

bool Exs_Platform::Exs_TranslateMessage(void* msg) {
    return TranslateMessage(static_cast<MSG*>(msg)) != 0;
}

bool Exs_Platform::Exs_DispatchMessage(void* msg) {
    return DispatchMessage(static_cast<MSG*>(msg)) != 0;
}

void Exs_Platform::Exs_PostQuitMessage(Exs_int32 exitCode) {
    PostQuitMessage(exitCode);
}
#endif

} // namespace Platform
} // namespace Internal
} // namespace Exs
