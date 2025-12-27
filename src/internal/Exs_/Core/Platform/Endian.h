// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_ENDIAN_H
#define EXS_ENDIAN_H

#include <cstdint>

namespace Exs_ {
namespace Platform {

enum class ExsEndianness {
    Little,
    Big,
    Unknown
};

class ExsEndian {
public:
    static ExsEndianness ExsGetSystemEndianness();
    
    template<typename T>
    static T ExsSwapBytes(T value) {
        static_assert(std::is_arithmetic<T>::value, "Type must be arithmetic");
        
        union {
            T value;
            uint8_t bytes[sizeof(T)];
        } src, dst;
        
        src.value = value;
        
        for (size_t i = 0; i < sizeof(T); i++) {
            dst.bytes[i] = src.bytes[sizeof(T) - i - 1];
        }
        
        return dst.value;
    }
    
    template<typename T>
    static T ExsToBigEndian(T value) {
        if (ExsGetSystemEndianness() == ExsEndianness::Little) {
            return ExsSwapBytes(value);
        }
        return value;
    }
    
    template<typename T>
    static T ExsToLittleEndian(T value) {
        if (ExsGetSystemEndianness() == ExsEndianness::Big) {
            return ExsSwapBytes(value);
        }
        return value;
    }
    
    template<typename T>
    static T ExsFromBigEndian(T value) {
        return ExsToBigEndian(value);
    }
    
    template<typename T>
    static T ExsFromLittleEndian(T value) {
        return ExsToLittleEndian(value);
    }
    
    static uint16_t ExsNetworkToHost16(uint16_t value);
    static uint32_t ExsNetworkToHost32(uint32_t value);
    static uint64_t ExsNetworkToHost64(uint64_t value);
    
    static uint16_t ExsHostToNetwork16(uint16_t value);
    static uint32_t ExsHostToNetwork32(uint32_t value);
    static uint64_t ExsHostToNetwork64(uint64_t value);
    
private:
    ExsEndian() = delete;
};

} // namespace Platform
} // namespace Exs_

#endif // EXS_ENDIAN_H
