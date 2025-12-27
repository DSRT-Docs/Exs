// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_POOL_ALLOCATOR_H
#define EXS_POOL_ALLOCATOR_H

#include "AllocatorBase.h"
#include "../../Core/Debug/Debug.h"
#include <atomic>

namespace Exs_ {
namespace Memory {

// Exs_ pool allocator for fixed-size allocations
class ExsPoolAllocator final : public ExsIAllocator {
private:
    // Chunk structure
    struct EXS_ALIGNAS(8) ExsChunk {
        ExsChunk* next;
        uint32_t magic;
        uint32_t poolIndex;
        
        static constexpr uint32_t EXS_CHUNK_MAGIC = 0x45585343; // "EXSC"
        static constexpr uint32_t EXS_CHUNK_FREE_MAGIC = 0x46524545; // "FREE"
    };
    
    // Pool block structure
    struct ExsPoolBlock {
        void* memory;
        size_t size;
        ExsPoolBlock* next;
        
        ExsPoolBlock() : memory(nullptr), size(0), next(nullptr) {}
    };
    
    // Configuration
    size_t chunkSize;
    size_t chunkCount;
    size_t alignment;
    size_t totalSize;
    
    // Memory management
    ExsPoolBlock* firstBlock;
    ExsPoolBlock* currentBlock;
    ExsChunk* freeList;
    
    // Statistics
    mutable std::atomic<ExsAllocationStats> stats;
    std::atomic<size_t> allocatedChunks;
    std::atomic<size_t> freeChunks;
    
    // Metadata
    char allocatorName[64];
    bool threadSafe;
    bool initialized;
    
    // Internal methods
    bool ExsInitializeBlock(ExsPoolBlock* block);
    void ExsDestroyBlock(ExsPoolBlock* block);
    
    ExsChunk* ExsAllocateFromPool();
    void ExsDeallocateToPool(ExsChunk* chunk);
    
    bool ExsIsValidChunk(const ExsChunk* chunk) const noexcept;
    bool ExsIsChunkInPool(const void* ptr) const noexcept;
    
    void ExsLock() const;
    void ExsUnlock() const;
    
    // Mutex for thread safety
    mutable void* mutex;
    
public:
    // Construction/Destruction
    ExsPoolAllocator(
        size_t chunkSize, 
        size_t chunkCount, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsPoolAllocator"
    );
    
    ~ExsPoolAllocator();
    
    // Non-copyable, non-movable
    ExsPoolAllocator(const ExsPoolAllocator&) = delete;
    ExsPoolAllocator& operator=(const ExsPoolAllocator&) = delete;
    ExsPoolAllocator(ExsPoolAllocator&&) = delete;
    ExsPoolAllocator& operator=(ExsPoolAllocator&&) = delete;
    
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
    
    // Pool-specific methods
    size_t ExsGetChunkSize() const noexcept { return chunkSize; }
    size_t ExsGetChunkCount() const noexcept { return chunkCount; }
    size_t ExsGetAllocatedChunks() const noexcept { return allocatedChunks.load(); }
    size_t ExsGetFreeChunks() const noexcept { return freeChunks.load(); }
    
    float ExsGetFragmentation() const noexcept;
    float ExsGetUtilization() const noexcept;
    
    void ExsDefragment();
    void ExsShrinkToFit();
    
    bool ExsAddMoreChunks(size_t additionalChunks);
    bool ExsResize(size_t newChunkCount);
    
    void ExsClear();
    
    // Factory methods
    static ExsPoolAllocator* ExsCreate(
        size_t chunkSize, 
        size_t chunkCount, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsPoolAllocator"
    );
    
    static void ExsDestroy(ExsPoolAllocator* allocator);
    
    // Validation
    bool ExsIsInitialized() const noexcept { return initialized; }
    bool ExsIsEmpty() const noexcept { return allocatedChunks.load() == 0; }
    bool ExsIsFull() const noexcept { return freeChunks.load() == 0; }
    
private:
    // Memory allocation callbacks
    static void* ExsAllocateMemory(size_t size, size_t alignment);
    static void ExsFreeMemory(void* ptr, size_t size);
    
    // Debug patterns
    void ExsFillChunkWithPattern(ExsChunk* chunk, ExsMemoryUtils::ExsMemoryPattern pattern);
    bool ExsCheckChunkPattern(const ExsChunk* chunk, ExsMemoryUtils::ExsMemoryPattern pattern);
    
    // Statistics helpers
    void ExsUpdateStatsAllocation(size_t size);
    void ExsUpdateStatsDeallocation(size_t size);
    void ExsUpdateStatsFailure();
};

// Exs_ pool allocator traits
template<>
struct ExsAllocatorTraits<ExsPoolAllocator> {
    using value_type = void;
    using pointer = void*;
    using const_pointer = const void*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    
    template<typename U>
    struct rebind {
        using other = ExsPoolAllocator;
    };
};

// Exs_ typed pool allocator
template<typename T, size_t ChunkCount = 256, size_t Alignment = alignof(T)>
class ExsTypedPoolAllocator {
private:
    ExsPoolAllocator* pool;
    
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    
    ExsTypedPoolAllocator() 
        : pool(ExsPoolAllocator::ExsCreate(sizeof(T), ChunkCount, Alignment)) {}
    
    ~ExsTypedPoolAllocator() {
        if (pool) {
            ExsPoolAllocator::ExsDestroy(pool);
        }
    }
    
    // Non-copyable
    ExsTypedPoolAllocator(const ExsTypedPoolAllocator&) = delete;
    ExsTypedPoolAllocator& operator=(const ExsTypedPoolAllocator&) = delete;
    
    // Movable
    ExsTypedPoolAllocator(ExsTypedPoolAllocator&& other) noexcept 
        : pool(other.pool) {
        other.pool = nullptr;
    }
    
    ExsTypedPoolAllocator& operator=(ExsTypedPoolAllocator&& other) noexcept {
        if (this != &other) {
            if (pool) {
                ExsPoolAllocator::ExsDestroy(pool);
            }
            pool = other.pool;
            other.pool = nullptr;
        }
        return *this;
    }
    
    pointer allocate(size_type n) {
        if (pool && n == 1) {
            return static_cast<pointer>(pool->ExsAllocate(sizeof(T), alignof(T)));
        }
        return static_cast<pointer>(::operator new(n * sizeof(T)));
    }
    
    void deallocate(pointer p, size_type n) {
        if (pool && n == 1) {
            pool->ExsDeallocate(p);
        } else {
            ::operator delete(p);
        }
    }
    
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }
    
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }
    
    size_type max_size() const noexcept {
        return size_type(-1) / sizeof(T);
    }
    
    ExsPoolAllocator* get_pool() const noexcept { return pool; }
    
    bool operator==(const ExsTypedPoolAllocator& other) const noexcept {
        return pool == other.pool;
    }
    
    bool operator!=(const ExsTypedPoolAllocator& other) const noexcept {
        return !(*this == other);
    }
};

} // namespace Memory
} // namespace Exs_

#endif // EXS_POOL_ALLOCATOR_H
