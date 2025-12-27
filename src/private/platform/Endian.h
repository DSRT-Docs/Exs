// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once
#include <cstdint>
#include <cstring>

namespace Exs {
namespace Platform {

class Exs_Endian {
public:
    enum class Exs_Endianness {
        Exs_Little,
        Exs_Big,
        Exs_Middle,  // PDP-endian (rare)
        Exs_Unknown
    };
    
    // Detect system endianness
    static Exs_Endianness Exs_GetSystemEndianness();
    static const char* Exs_GetEndiannessName();
    
    // Byte swapping functions
    static uint16_t Exs_Swap16(uint16_t value);
    static uint32_t Exs_Swap32(uint32_t value);
    static uint64_t Exs_Swap64(uint64_t value);
    
    // Generic swap for any integer type
    template<typename T>
    static T Exs_Swap(T value);
    
    // Convert to specific endianness from host
    static uint16_t Exs_ToLittleEndian16(uint16_t value);
    static uint32_t Exs_ToLittleEndian32(uint32_t value);
    static uint64_t Exs_ToLittleEndian64(uint64_t value);
    
    static uint16_t Exs_ToBigEndian16(uint16_t value);
    static uint32_t Exs_ToBigEndian32(uint32_t value);
    static uint64_t Exs_ToBigEndian64(uint64_t value);
    
    // Convert from specific endianness to host
    static uint16_t Exs_FromLittleEndian16(uint16_t value);
    static uint32_t Exs_FromLittleEndian32(uint32_t value);
    static uint64_t Exs_FromLittleEndian64(uint64_t value);
    
    static uint16_t Exs_FromBigEndian16(uint16_t value);
    static uint32_t Exs_FromBigEndian32(uint32_t value);
    static uint64_t Exs_FromBigEndian64(uint64_t value);
    
    // Network byte order (big endian) conversion
    static uint16_t Exs_HTONS(uint16_t host_short);
    static uint32_t Exs_HTONL(uint32_t host_long);
    static uint64_t Exs_HTONLL(uint64_t host_long_long);
    
    static uint16_t Exs_NTOHS(uint16_t network_short);
    static uint32_t Exs_NTOHL(uint32_t network_long);
    static uint64_t Exs_NTOHLL(uint64_t network_long_long);
    
    // Memory-based swapping
    static void Exs_Swap16(void* data, size_t count = 1);
    static void Exs_Swap32(void* data, size_t count = 1);
    static void Exs_Swap64(void* data, size_t count = 1);
    
    // Endian-aware reading/writing
    template<typename T>
    static T Exs_ReadLittleEndian(const void* data);
    
    template<typename T>
    static T Exs_ReadBigEndian(const void* data);
    
    template<typename T>
    static void Exs_WriteLittleEndian(void* data, T value);
    
    template<typename T>
    static void Exs_WriteBigEndian(void* data, T value);
    
    // Array conversion
    static void Exs_ConvertArrayToLittleEndian(void* data, size_t element_size, size_t count);
    static void Exs_ConvertArrayToBigEndian(void* data, size_t element_size, size_t count);
    
    // Debug functions
    static void Exs_DumpEndianInfo();
    static void Exs_TestEndianConversion();
};

} // namespace Platform
} // namespace Exs
