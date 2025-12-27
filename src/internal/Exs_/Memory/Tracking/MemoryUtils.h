// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_MEMORY_UTILS_H
#define EXS_MEMORY_UTILS_H

#include "../Allocator/AllocatorBase.h"
#include "../../Core/Macros.h"
#include <type_traits>

namespace Exs_ {
namespace Memory {

// Exs_ memory initialization utilities
namespace ExsMemoryUtils {

    // Memory initialization
    template<typename T>
    void ExsConstruct(T* ptr) {
        static_assert(std::is_default_constructible<T>::value,
                     "Type must be default constructible");
        ::new (static_cast<void*>(ptr)) T();
    }
    
    template<typename T, typename... Args>
    void ExsConstruct(T* ptr, Args&&... args) {
        static_assert(std::is_constructible<T, Args...>::value,
                     "Type must be constructible with given arguments");
        ::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
    }
    
    template<typename T>
    void ExsDestroy(T* ptr) {
        static_assert(std::is_destructible<T>::value,
                     "Type must be destructible");
        ptr->~T();
    }
    
    // Array construction/destruction
    template<typename T>
    void ExsConstructArray(T* ptr, size_t count) {
        static_assert(std::is_default_constructible<T>::value,
                     "Type must be default constructible");
        for (size_t i = 0; i < count; ++i) {
            ::new (static_cast<void*>(ptr + i)) T();
        }
    }
    
    template<typename T>
    void ExsDestroyArray(T* ptr, size_t count) {
        static_assert(std::is_destructible<T>::value,
                     "Type must be destructible");
        for (size_t i = 0; i < count; ++i) {
            ptr[i].~T();
        }
    }
    
    // Memory copying with type safety
    template<typename T>
    void ExsCopyConstruct(T* dest, const T* src, size_t count) {
        static_assert(std::is_copy_constructible<T>::value,
                     "Type must be copy constructible");
        for (size_t i = 0; i < count; ++i) {
            ::new (static_cast<void*>(dest + i)) T(src[i]);
        }
    }
    
    template<typename T>
    void ExsMoveConstruct(T* dest, T* src, size_t count) {
        static_assert(std::is_move_constructible<T>::value,
                     "Type must be move constructible");
        for (size_t i = 0; i < count; ++i) {
            ::new (static_cast<void*>(dest + i)) T(std::move(src[i]));
        }
    }
    
    // Memory utilities for primitive types
    template<typename T>
    void ExsZeroMemory(T* ptr, size_t count = 1) {
        static_assert(std::is_trivial<T>::value,
                     "Type must be trivial for zero memory");
        ExsMemSet(ptr, 0, count * sizeof(T));
    }
    
    template<typename T>
    void ExsFillMemory(T* ptr, const T& value, size_t count = 1) {
        static_assert(std::is_trivial<T>::value,
                     "Type must be trivial for fill memory");
        for (size_t i = 0; i < count; ++i) {
            ptr[i] = value;
        }
    }
    
    // Memory comparison with type safety
    template<typename T>
    bool ExsCompareMemory(const T* a, const T* b, size_t count = 1) {
        static_assert(std::is_trivial<T>::value,
                     "Type must be trivial for memory comparison");
        return ExsMemCompare(a, b, count * sizeof(T)) == 0;
    }
    
    // Memory swapping
    template<typename T>
    void ExsSwapMemory(T* a, T* b, size_t count = 1) {
        static_assert(std::is_trivial<T>::value,
                     "Type must be trivial for memory swap");
        for (size_t i = 0; i < count; ++i) {
            T temp = a[i];
            a[i] = b[i];
            b[i] = temp;
        }
    }
    
    // Memory alignment calculation
    template<typename T>
    constexpr size_t ExsCalculateAlignedSize(size_t count) {
        constexpr size_t alignment = alignof(T);
        constexpr size_t size = sizeof(T);
        
        size_t total = count * size;
        size_t padding = (alignment - (total % alignment)) % alignment;
        
        return total + padding;
    }
    
    template<typename T>
    constexpr size_t ExsCalculatePaddingForType(size_t offset) {
        constexpr size_t alignment = alignof(T);
        return ExsCalculatePadding(offset, alignment);
    }
    
    // Memory pattern utilities
    template<typename T>
    void ExsFillWithPattern(T* ptr, size_t count, uint64_t pattern) {
        uint8_t* bytePtr = reinterpret_cast<uint8_t*>(ptr);
        const uint8_t* patternPtr = reinterpret_cast<const uint8_t*>(&pattern);
        const size_t patternSize = sizeof(pattern);
        
        for (size_t i = 0; i < count * sizeof(T); ++i) {
            bytePtr[i] = patternPtr[i % patternSize];
        }
    }
    
    // Memory validation for arrays
    template<typename T>
    bool ExsValidateArray(const T* ptr, size_t count) {
        if (ptr == nullptr && count > 0) {
            return false;
        }
        
        // Check for null pointers in array (if T is pointer type)
        if constexpr (std::is_pointer<T>::value) {
            for (size_t i = 0; i < count; ++i) {
                if (ptr[i] == nullptr) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // Memory range utilities
    template<typename T>
    bool ExsIsInRange(const T* ptr, const T* rangeStart, const T* rangeEnd) {
        return ptr >= rangeStart && ptr < rangeEnd;
    }
    
    template<typename T>
    bool ExsIsInRange(const T* ptr, const T* rangeStart, size_t count) {
        return ptr >= rangeStart && ptr < rangeStart + count;
    }
    
    // Memory block utilities
    struct ExsMemoryBlock {
        void* ptr;
        size_t size;
        size_t alignment;
        
        ExsMemoryBlock() : ptr(nullptr), size(0), alignment(0) {}
        ExsMemoryBlock(void* p, size_t s, size_t a = 8) : ptr(p), size(s), alignment(a) {}
        
        bool ExsIsValid() const { return ptr != nullptr && size > 0; }
        bool ExsIsAligned() const { return ExsIsAligned(ptr, alignment); }
        
        template<typename T>
        T* ExsAs() { return static_cast<T*>(ptr); }
        
        template<typename T>
        const T* ExsAs() const { return static_cast<const T*>(ptr); }
    };
    
    ExsMemoryBlock ExsAllocateAlignedBlock(size_t size, size_t alignment);
    void ExsFreeAlignedBlock(ExsMemoryBlock& block);
    
    ExsMemoryBlock ExsAllocatePageAlignedBlock(size_t size);
    void ExsFreePageAlignedBlock(ExsMemoryBlock& block);
    
    // Memory pool utilities
    template<typename T, size_t Capacity>
    class ExsFixedMemoryPool {
    private:
        union ExsPoolSlot {
            T value;
            ExsPoolSlot* next;
            
            ExsPoolSlot() : next(nullptr) {}
            ~ExsPoolSlot() {}
        };
        
        ExsPoolSlot memory[Capacity];
        ExsPoolSlot* freeList;
        size_t usedSlots;
        
    public:
        ExsFixedMemoryPool() : freeList(nullptr), usedSlots(0) {
            // Initialize free list
            for (size_t i = 0; i < Capacity; ++i) {
                memory[i].next = freeList;
                freeList = &memory[i];
            }
        }
        
        ~ExsFixedMemoryPool() {
            // Destroy all allocated objects
            for (size_t i = 0; i < Capacity; ++i) {
                if (memory[i].next == nullptr) { // Slot is used
                    memory[i].value.~T();
                }
            }
        }
        
        template<typename... Args>
        T* ExsAllocate(Args&&... args) {
            if (freeList == nullptr) {
                return nullptr;
            }
            
            ExsPoolSlot* slot = freeList;
            freeList = slot->next;
            
            ::new (&slot->value) T(std::forward<Args>(args)...);
            ++usedSlots;
            
            return &slot->value;
        }
        
        void ExsDeallocate(T* ptr) {
            if (ptr == nullptr) {
                return;
            }
            
            // Find the slot
            ExsPoolSlot* slot = reinterpret_cast<ExsPoolSlot*>(
                reinterpret_cast<uint8_t*>(ptr) - offsetof(ExsPoolSlot, value)
            );
            
            // Destroy the object
            ptr->~T();
            
            // Add to free list
            slot->next = freeList;
            freeList = slot;
            
            --usedSlots;
        }
        
        size_t ExsGetUsedSlots() const { return usedSlots; }
        size_t ExsGetFreeSlots() const { return Capacity - usedSlots; }
        size_t ExsGetCapacity() const { return Capacity; }
        
        bool ExsIsFull() const { return usedSlots == Capacity; }
        bool ExsIsEmpty() const { return usedSlots == 0; }
    };
    
    // Memory arena utilities
    class ExsScopedMemoryArena {
    private:
        ExsIAllocator* allocator;
        void* marker;
        
    public:
        explicit ExsScopedMemoryArena(ExsIAllocator* alloc) 
            : allocator(alloc), marker(nullptr) {
            if (allocator) {
                // Get current marker if allocator supports it
                if (auto stackAlloc = dynamic_cast<ExsStackAllocator*>(allocator)) {
                    marker = reinterpret_cast<void*>(
                        stackAlloc->ExsGetMarker().offset
                    );
                }
            }
        }
        
        ~ExsScopedMemoryArena() {
            if (allocator && marker) {
                if (auto stackAlloc = dynamic_cast<ExsStackAllocator*>(allocator)) {
                    ExsStackAllocator::ExsStackMarker stackMarker;
                    stackMarker.offset = reinterpret_cast<size_t>(marker);
                    stackAlloc->ExsRewindToMarker(stackMarker);
                }
            }
        }
        
        // Non-copyable
        ExsScopedMemoryArena(const ExsScopedMemoryArena&) = delete;
        ExsScopedMemoryArena& operator=(const ExsScopedMemoryArena&) = delete;
        
        // Movable
        ExsScopedMemoryArena(ExsScopedMemoryArena&& other) noexcept 
            : allocator(other.allocator), marker(other.marker) {
            other.allocator = nullptr;
            other.marker = nullptr;
        }
        
        ExsScopedMemoryArena& operator=(ExsScopedMemoryArena&& other) noexcept {
            if (this != &other) {
                allocator = other.allocator;
                marker = other.marker;
                other.allocator = nullptr;
                other.marker = nullptr;
            }
            return *this;
        }
        
        ExsIAllocator* ExsGetAllocator() const { return allocator; }
        void* ExsGetMarker() const { return marker; }
    };
    
    // Memory debugging utilities
    void ExsDumpMemoryInfo();
    void ExsDumpAllocatorInfo();
    void ExsCheckMemoryConsistency();
    
    // Memory profiling utilities
    struct ExsMemoryProfile {
        size_t totalAllocated;
        size_t totalFreed;
        size_t currentUsage;
        size_t peakUsage;
        size_t allocationCount;
        size_t deallocationCount;
        double fragmentation;
        
        void ExsReset() {
            totalAllocated = 0;
            totalFreed = 0;
            currentUsage = 0;
            peakUsage = 0;
            allocationCount = 0;
            deallocationCount = 0;
            fragmentation = 0.0;
        }
    };
    
    ExsMemoryProfile ExsGetMemoryProfile();
    void ExsResetMemoryProfile();
    
} // namespace ExsMemoryUtils

} // namespace Memory
} // namespace Exs_

#endif // EXS_MEMORY_UTILS_H
