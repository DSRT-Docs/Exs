// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_ARENA_ALLOCATOR_H
#define EXS_ARENA_ALLOCATOR_H

#include "AllocatorBase.h"
#include "../../Core/Debug/Debug.h"
#include <atomic>

namespace Exs_ {
namespace Memory {

// Exs_ arena block structure
struct ExsArenaBlock {
    void* memory;
    size_t size;
    size_t used;
    ExsArenaBlock* next;
    uint32_t blockId;
    uint32_t magic;
    
    static constexpr uint32_t EXS_ARENA_BLOCK_MAGIC = 0x45585342; // "EXSB"
    
    ExsArenaBlock() 
        : memory(nullptr), size(0), used(0), next(nullptr), 
          blockId(0), magic(EXS_ARENA_BLOCK_MAGIC) {}
};

// Exs_ arena allocator for fast allocations with bulk deallocation
class ExsArenaAllocator final : public ExsIAllocator {
private:
    // Configuration
    size_t blockSize;
    size_t alignment;
    
    // Block management
    ExsArenaBlock* firstBlock;
    ExsArenaBlock* currentBlock;
    std::atomic<uint32_t> blockCounter;
    
    // Statistics
    mutable std::atomic<ExsAllocationStats> stats;
    
    // Metadata
    char allocatorName[64];
    bool threadSafe;
    
    // Mutex for thread safety
    mutable void* mutex;
    
    // Internal methods
    ExsArenaBlock* ExsCreateBlock(size_t size);
    void ExsDestroyBlock(ExsArenaBlock* block);
    
    void* ExsAllocateFromBlock(ExsArenaBlock* block, size_t size, size_t alignment);
    
    void ExsLock() const;
    void ExsUnlock() const;
    
public:
    // Construction/Destruction
    ExsArenaAllocator(
        size_t blockSize = 65536, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsArenaAllocator"
    );
    
    ~ExsArenaAllocator();
    
    // Non-copyable, non-movable
    ExsArenaAllocator(const ExsArenaAllocator&) = delete;
    ExsArenaAllocator& operator=(const ExsArenaAllocator&) = delete;
    ExsArenaAllocator(ExsArenaAllocator&&) = delete;
    ExsArenaAllocator& operator=(ExsArenaAllocator&&) = delete;
    
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
    
    // Arena-specific methods
    void ExsClear();
    void ExsReset();
    
    size_t ExsGetBlockSize() const noexcept { return blockSize; }
    size_t ExsGetBlockCount() const noexcept;
    size_t ExsGetUsedBlocks() const noexcept;
    
    float ExsGetUtilization() const noexcept;
    float ExsGetFragmentation() const noexcept;
    
    bool ExsIsEmpty() const noexcept;
    
    void ExsSetBlockSize(size_t newBlockSize);
    
    // Factory methods
    static ExsArenaAllocator* ExsCreate(
        size_t blockSize = 65536, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsArenaAllocator"
    );
    
    static void ExsDestroy(ExsArenaAllocator* allocator);
    
private:
    // Allocation tracking
    struct ExsArenaAllocationInfo {
        ExsArenaBlock* block;
        size_t offset;
        size_t size;
    };
    
    // Block management helpers
    bool ExsIsValidBlock(const ExsArenaBlock* block) const;
    ExsArenaBlock* ExsFindBlockForPointer(const void* ptr) const;
    
    // Statistics helpers
    void ExsUpdateStatsAllocation(size_t size);
    void ExsRecalculateStats();
    
    // Debug
    void ExsFillBlockWithPattern(ExsArenaBlock* block, ExsMemoryUtils::ExsMemoryPattern pattern);
};

// Exs_ scoped arena allocator
class ExsScopedArenaAllocator {
private:
    ExsArenaAllocator* allocator;
    size_t initialBlockCount;
    
public:
    explicit ExsScopedArenaAllocator(ExsArenaAllocator* alloc)
        : allocator(alloc), initialBlockCount(alloc->ExsGetBlockCount()) {}
    
    ~ExsScopedArenaAllocator() {
        if (allocator && allocator->ExsGetBlockCount() > initialBlockCount) {
            allocator->ExsClear();
        }
    }
    
    // Non-copyable
    ExsScopedArenaAllocator(const ExsScopedArenaAllocator&) = delete;
    ExsScopedArenaAllocator& operator=(const ExsScopedArenaAllocator&) = delete;
    
    // Movable
    ExsScopedArenaAllocator(ExsScopedArenaAllocator&& other) noexcept 
        : allocator(other.allocator), initialBlockCount(other.initialBlockCount) {
        other.allocator = nullptr;
    }
    
    ExsScopedArenaAllocator& operator=(ExsScopedArenaAllocator&& other) noexcept {
        if (this != &other) {
            allocator = other.allocator;
            initialBlockCount = other.initialBlockCount;
            other.allocator = nullptr;
        }
        return *this;
    }
    
    ExsArenaAllocator* ExsGetAllocator() const noexcept { return allocator; }
    size_t ExsGetInitialBlockCount() const noexcept { return initialBlockCount; }
    
    void ExsRelease() { allocator = nullptr; }
};

} // namespace Memory
} // namespace Exs_

#endif // EXS_ARENA_ALLOCATOR_H
