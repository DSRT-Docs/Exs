// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_MEMORY_TRACKER_H
#define EXS_MEMORY_TRACKER_H

#include "../Allocator/AllocatorBase.h"
#include "../../Core/Debug/Debug.h"
#include "../../Core/Platform/Platform.h"
#include <atomic>
#include <cstddef>

namespace Exs_ {
namespace Memory {

// Exs_ memory leak detection
class ExsMemoryTracker {
public:
    struct ExsAllocationRecord {
        void* ptr;
        size_t size;
        size_t alignment;
        const char* file;
        int line;
        uint64_t timestamp;
        uint32_t allocationId;
        uint32_t threadId;
        
        ExsAllocationRecord* next;
        ExsAllocationRecord* prev;
    };
    
    struct ExsMemorySnapshot {
        size_t totalAllocations;
        size_t totalBytes;
        size_t peakAllocations;
        size_t peakBytes;
        uint64_t snapshotTime;
        
        ExsAllocationRecord* allocations;
    };
    
    // Singleton instance
    static ExsMemoryTracker& ExsGetInstance();
    
    // Initialization
    void ExsInitialize();
    void ExsShutdown();
    
    // Allocation tracking
    void ExsTrackAllocation(
        void* ptr, 
        size_t size, 
        size_t alignment,
        const char* file = nullptr,
        int line = 0
    );
    
    void ExsTrackDeallocation(void* ptr);
    
    // Snapshot management
    ExsMemorySnapshot ExsCreateSnapshot() const;
    void ExsCompareSnapshots(
        const ExsMemorySnapshot& before,
        const ExsMemorySnapshot& after
    ) const;
    
    void ExsClearSnapshots();
    
    // Leak detection
    size_t ExsDetectLeaks() const;
    void ExsDumpLeaks() const;
    void ExsDumpAllocations() const;
    
    // Statistics
    size_t ExsGetTotalAllocations() const;
    size_t ExsGetTotalDeallocations() const;
    size_t ExsGetCurrentAllocations() const;
    size_t ExsGetTotalBytesAllocated() const;
    size_t ExsGetTotalBytesFreed() const;
    size_t ExsGetCurrentBytes() const;
    size_t ExsGetPeakBytes() const;
    
    // Configuration
    void ExsSetEnabled(bool enabled);
    bool ExsIsEnabled() const;
    
    void ExsSetTrackFileLine(bool track);
    bool ExsIsTrackingFileLine() const;
    
    void ExsSetBreakOnLeak(bool breakOnLeak);
    bool ExsIsBreakingOnLeak() const;
    
    void ExsSetDumpOnExit(bool dumpOnExit);
    bool ExsIsDumpingOnExit() const;
    
    // Filtering
    void ExsAddFilter(const char* filter);
    void ExsRemoveFilter(const char* filter);
    void ExsClearFilters();
    
    bool ExsIsFiltered(const char* file) const;
    
private:
    ExsMemoryTracker();
    ~ExsMemoryTracker();
    
    // Non-copyable
    ExsMemoryTracker(const ExsMemoryTracker&) = delete;
    ExsMemoryTracker& operator=(const ExsMemoryTracker&) = delete;
    
    // Internal methods
    ExsAllocationRecord* ExsCreateRecord(
        void* ptr, 
        size_t size, 
        size_t alignment,
        const char* file,
        int line
    );
    
    void ExsDestroyRecord(ExsAllocationRecord* record);
    
    void ExsAddRecord(ExsAllocationRecord* record);
    void ExsRemoveRecord(ExsAllocationRecord* record);
    
    ExsAllocationRecord* ExsFindRecord(void* ptr) const;
    
    void ExsLock() const;
    void ExsUnlock() const;
    
    // Data members
    mutable void* mutex;
    ExsAllocationRecord* allocationList;
    
    std::atomic<size_t> totalAllocations;
    std::atomic<size_t> totalDeallocations;
    std::atomic<size_t> totalBytesAllocated;
    std::atomic<size_t> totalBytesFreed;
    std::atomic<size_t> peakBytes;
    
    std::atomic<uint32_t> allocationCounter;
    
    bool enabled;
    bool trackFileLine;
    bool breakOnLeak;
    bool dumpOnExit;
    
    struct FilterNode {
        char* filter;
        FilterNode* next;
    };
    
    FilterNode* filterList;
};

// Exs_ tracked allocator wrapper
class ExsTrackedAllocator : public ExsIAllocator {
private:
    ExsIAllocator* baseAllocator;
    ExsMemoryTracker* tracker;
    const char* name;
    
public:
    ExsTrackedAllocator(
        ExsIAllocator* allocator, 
        const char* allocatorName = "TrackedAllocator"
    );
    
    ~ExsTrackedAllocator();
    
    // ExsIAllocator implementation
    void* ExsAllocate(size_t size, size_t alignment = 8) override;
    void ExsDeallocate(void* ptr) override;
    void* ExsReallocate(void* ptr, size_t newSize, size_t alignment = 8) override;
    
    ExsAllocationStats ExsGetStats() const override;
    void ExsResetStats() override;
    
    const char* ExsGetName() const override;
    size_t ExsGetTotalSize() const override;
    size_t ExsGetUsedSize() const override;
    size_t ExsGetFreeSize() const override;
    
    bool ExsOwnsPointer(const void* ptr) const override;
    bool ExsIsValidPointer(const void* ptr) const override;
    bool ExsCanAllocate(size_t size, size_t alignment = 8) const override;
    
    void ExsDumpMemoryMap() const override;
    void ExsValidate() const override;
    
    void ExsSetName(const char* name) override;
    void ExsSetThreadSafe(bool threadSafe) override;
    bool ExsIsThreadSafe() const override;
    
    // Tracking methods
    void ExsEnableTracking(bool enable);
    bool ExsIsTrackingEnabled() const;
    
    void ExsSetTracker(ExsMemoryTracker* newTracker);
    ExsMemoryTracker* ExsGetTracker() const;
    
    ExsIAllocator* ExsGetBaseAllocator() const;
    
private:
    bool trackingEnabled;
};

// Exs_ allocation macros
#if EXS_FEATURE_MEMORY_TRACKING

    #define EXS_TRACK_ALLOCATION(size, alignment) \
        Exs_::Memory::ExsMemoryTracker::ExsGetInstance().ExsTrackAllocation( \
            nullptr, size, alignment, __FILE__, __LINE__)
    
    #define EXS_TRACK_DEALLOCATION(ptr) \
        Exs_::Memory::ExsMemoryTracker::ExsGetInstance().ExsTrackDeallocation(ptr)
    
    #define EXS_NEW(type, allocator, ...) \
        [&]() -> type* { \
            void* ptr = allocator->ExsAllocate(sizeof(type), alignof(type)); \
            EXS_TRACK_ALLOCATION(sizeof(type), alignof(type)); \
            return new (ptr) type(__VA_ARGS__); \
        }()
    
    #define EXS_DELETE(ptr, allocator) \
        do { \
            if (ptr) { \
                ptr->~decltype(*ptr)(); \
                allocator->ExsDeallocate(ptr); \
                EXS_TRACK_DEALLOCATION(ptr); \
                ptr = nullptr; \
            } \
        } while(0)
    
    #define EXS_ALLOCATE_TRACKED(allocator, size, alignment) \
        [&]() -> void* { \
            void* ptr = allocator->ExsAllocate(size, alignment); \
            EXS_TRACK_ALLOCATION(size, alignment); \
            return ptr; \
        }()
    
    #define EXS_DEALLOCATE_TRACKED(ptr, allocator) \
        do { \
            if (ptr) { \
                allocator->ExsDeallocate(ptr); \
                EXS_TRACK_DEALLOCATION(ptr); \
                ptr = nullptr; \
            } \
        } while(0)

#else

    #define EXS_TRACK_ALLOCATION(size, alignment) ((void)0)
    #define EXS_TRACK_DEALLOCATION(ptr) ((void)0)
    
    #define EXS_NEW(type, allocator, ...) \
        new (allocator->ExsAllocate(sizeof(type), alignof(type))) type(__VA_ARGS__)
    
    #define EXS_DELETE(ptr, allocator) \
        do { \
            if (ptr) { \
                ptr->~decltype(*ptr)(); \
                allocator->ExsDeallocate(ptr); \
                ptr = nullptr; \
            } \
        } while(0)
    
    #define EXS_ALLOCATE_TRACKED(allocator, size, alignment) \
        allocator->ExsAllocate(size, alignment)
    
    #define EXS_DEALLOCATE_TRACKED(ptr, allocator) \
        do { \
            if (ptr) { \
                allocator->ExsDeallocate(ptr); \
                ptr = nullptr; \
            } \
        } while(0)

#endif

} // namespace Memory
} // namespace Exs_

#endif // EXS_MEMORY_TRACKER_H
