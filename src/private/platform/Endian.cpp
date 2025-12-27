#include "Endian.h"
#include <iostream>

namespace Exs {
namespace Platform {

Exs_Endian::Exs_Endianness Exs_Endian::Exs_GetSystemEndianness() {
    constexpr uint32_t test = 0x01020304;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&test);
    
    if (bytes[0] == 0x01 && bytes[1] == 0x02 && 
        bytes[2] == 0x03 && bytes[3] == 0x04) {
        return Exs_Endianness::Exs_Big;
    }
    else if (bytes[0] == 0x04 && bytes[1] == 0x03 && 
             bytes[2] == 0x02 && bytes[3] == 0x01) {
        return Exs_Endianness::Exs_Little;
    }
    else if (bytes[0] == 0x02 && bytes[1] == 0x01 && 
             bytes[2] == 0x04 && bytes[3] == 0x03) {
        return Exs_Endianness::Exs_Middle;
    }
    
    return Exs_Endianness::Exs_Unknown;
}

const char* Exs_Endian::Exs_GetEndiannessName() {
    switch (Exs_GetSystemEndianness()) {
        case Exs_Endianness::Exs_Little: return "Little Endian";
        case Exs_Endianness::Exs_Big: return "Big Endian";
        case Exs_Endianness::Exs_Middle: return "Middle Endian (PDP)";
        default: return "Unknown";
    }
}

uint16_t Exs_Endian::Exs_Swap16(uint16_t value) {
    return ((value & 0x00FF) << 8) |
           ((value & 0xFF00) >> 8);
}

uint32_t Exs_Endian::Exs_Swap32(uint32_t value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

uint64_t Exs_Endian::Exs_Swap64(uint64_t value) {
    return ((value & 0x00000000000000FFULL) << 56) |
           ((value & 0x000000000000FF00ULL) << 40) |
           ((value & 0x0000000000FF0000ULL) << 24) |
           ((value & 0x00000000FF000000ULL) << 8) |
           ((value & 0x000000FF00000000ULL) >> 8) |
           ((value & 0x0000FF0000000000ULL) >> 24) |
           ((value & 0x00FF000000000000ULL) >> 40) |
           ((value & 0xFF00000000000000ULL) >> 56);
}

template<typename T>
T Exs_Endian::Exs_Swap(T value) {
    static_assert(std::is_integral<T>::value, "Only integer types are supported");
    
    switch (sizeof(T)) {
        case 1: return value;
        case 2: return static_cast<T>(Exs_Swap16(static_cast<uint16_t>(value)));
        case 4: return static_cast<T>(Exs_Swap32(static_cast<uint32_t>(value)));
        case 8: return static_cast<T>(Exs_Swap64(static_cast<uint64_t>(value)));
        default:
            // For other sizes, use byte-by-byte swapping
            T result = 0;
            uint8_t* src = reinterpret_cast<uint8_t*>(&value);
            uint8_t* dst = reinterpret_cast<uint8_t*>(&result);
            
            for (size_t i = 0; i < sizeof(T); i++) {
                dst[i] = src[sizeof(T) - 1 - i];
            }
            return result;
    }
}

// Explicit template instantiations
template int8_t Exs_Endian::Exs_Swap<int8_t>(int8_t);
template uint8_t Exs_Endian::Exs_Swap<uint8_t>(uint8_t);
template int16_t Exs_Endian::Exs_Swap<int16_t>(int16_t);
template uint16_t Exs_Endian::Exs_Swap<uint16_t>(uint16_t);
template int32_t Exs_Endian::Exs_Swap<int32_t>(int32_t);
template uint32_t Exs_Endian::Exs_Swap<uint32_t>(uint32_t);
template int64_t Exs_Endian::Exs_Swap<int64_t>(int64_t);
template uint64_t Exs_Endian::Exs_Swap<uint64_t>(uint64_t);

uint16_t Exs_Endian::Exs_ToLittleEndian16(uint16_t value) {
#if defined(EXS_LITTLE_ENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return value;
#else
    return Exs_Swap16(value);
#endif
}

uint32_t Exs_Endian::Exs_ToLittleEndian32(uint32_t value) {
#if defined(EXS_LITTLE_ENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return value;
#else
    return Exs_Swap32(value);
#endif
}

uint64_t Exs_Endian::Exs_ToLittleEndian64(uint64_t value) {
#if defined(EXS_LITTLE_ENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return value;
#else
    return Exs_Swap64(value);
#endif
}

uint16_t Exs_Endian::Exs_ToBigEndian16(uint16_t value) {
#if defined(EXS_BIG_ENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return value;
#else
    return Exs_Swap16(value);
#endif
}

uint32_t Exs_Endian::Exs_ToBigEndian32(uint32_t value) {
#if defined(EXS_BIG_ENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return value;
#else
    return Exs_Swap32(value);
#endif
}

uint64_t Exs_Endian::Exs_ToBigEndian64(uint64_t value) {
#if defined(EXS_BIG_ENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return value;
#else
    return Exs_Swap64(value);
#endif
}

uint16_t Exs_Endian::Exs_FromLittleEndian16(uint16_t value) {
    return Exs_ToLittleEndian16(value); // Same operation
}

uint32_t Exs_Endian::Exs_FromLittleEndian32(uint32_t value) {
    return Exs_ToLittleEndian32(value);
}

uint64_t Exs_Endian::Exs_FromLittleEndian64(uint64_t value) {
    return Exs_ToLittleEndian64(value);
}

uint16_t Exs_Endian::Exs_FromBigEndian16(uint16_t value) {
    return Exs_ToBigEndian16(value);
}

uint32_t Exs_Endian::Exs_FromBigEndian32(uint32_t value) {
    return Exs_ToBigEndian32(value);
}

uint64_t Exs_Endian::Exs_FromBigEndian64(uint64_t value) {
    return Exs_ToBigEndian64(value);
}

uint16_t Exs_Endian::Exs_HTONS(uint16_t host_short) {
    return Exs_ToBigEndian16(host_short);
}

uint32_t Exs_Endian::Exs_HTONL(uint32_t host_long) {
    return Exs_ToBigEndian32(host_long);
}

uint64_t Exs_Endian::Exs_HTONLL(uint64_t host_long_long) {
    return Exs_ToBigEndian64(host_long_long);
}

uint16_t Exs_Endian::Exs_NTOHS(uint16_t network_short) {
    return Exs_FromBigEndian16(network_short);
}

uint32_t Exs_Endian::Exs_NTOHL(uint32_t network_long) {
    return Exs_FromBigEndian32(network_long);
}

uint64_t Exs_Endian::Exs_NTOHLL(uint64_t network_long_long) {
    return Exs_FromBigEndian64(network_long_long);
}

void Exs_Endian::Exs_Swap16(void* data, size_t count) {
    uint16_t* ptr = static_cast<uint16_t*>(data);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = Exs_Swap16(ptr[i]);
    }
}

void Exs_Endian::Exs_Swap32(void* data, size_t count) {
    uint32_t* ptr = static_cast<uint32_t*>(data);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = Exs_Swap32(ptr[i]);
    }
}

void Exs_Endian::Exs_Swap64(void* data, size_t count) {
    uint64_t* ptr = static_cast<uint64_t*>(data);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = Exs_Swap64(ptr[i]);
    }
}

template<typename T>
T Exs_Endian::Exs_ReadLittleEndian(const void* data) {
    T value;
    std::memcpy(&value, data, sizeof(T));
    
    if (Exs_GetSystemEndianness() != Exs_Endianness::Exs_Little) {
        value = Exs_Swap<T>(value);
    }
    
    return value;
}

template<typename T>
T Exs_Endian::Exs_ReadBigEndian(const void* data) {
    T value;
    std::memcpy(&value, data, sizeof(T));
    
    if (Exs_GetSystemEndianness() != Exs_Endianness::Exs_Big) {
        value = Exs_Swap<T>(value);
    }
    
    return value;
}

template<typename T>
void Exs_Endian::Exs_WriteLittleEndian(void* data, T value) {
    if (Exs_GetSystemEndianness() != Exs_Endianness::Exs_Little) {
        value = Exs_Swap<T>(value);
    }
    
    std::memcpy(data, &value, sizeof(T));
}

template<typename T>
void Exs_Endian::Exs_WriteBigEndian(void* data, T value) {
    if (Exs_GetSystemEndianness() != Exs_Endianness::Exs_Big) {
        value = Exs_Swap<T>(value);
    }
    
    std::memcpy(data, &value, sizeof(T));
}

void Exs_Endian::Exs_ConvertArrayToLittleEndian(void* data, size_t element_size, size_t count) {
    uint8_t* bytes = static_cast<uint8_t*>(data);
    
    if (Exs_GetSystemEndianness() == Exs_Endianness::Exs_Little) {
        return; // Already little endian
    }
    
    for (size_t i = 0; i < count; i++) {
        uint8_t* element = bytes + (i * element_size);
        
        // Reverse bytes
        for (size_t j = 0; j < element_size / 2; j++) {
            std::swap(element[j], element[element_size - 1 - j]);
        }
    }
}

void Exs_Endian::Exs_ConvertArrayToBigEndian(void* data, size_t element_size, size_t count) {
    uint8_t* bytes = static_cast<uint8_t*>(data);
    
    if (Exs_GetSystemEndianness() == Exs_Endianness::Exs_Big) {
        return; // Already big endian
    }
    
    for (size_t i = 0; i < count; i++) {
        uint8_t* element = bytes + (i * element_size);
        
        // Reverse bytes
        for (size_t j = 0; j < element_size / 2; j++) {
            std::swap(element[j], element[element_size - 1 - j]);
        }
    }
}

void Exs_Endian::Exs_DumpEndianInfo() {
    std::cout << "=== Exs Endian Info ===" << std::endl;
    std::cout << "System Endianness: " << Exs_GetEndiannessName() << std::endl;
    
    // Test value
    uint32_t test_value = 0x12345678;
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&test_value);
    
    std::cout << "Test value 0x12345678 in memory: ";
    for (size_t i = 0; i < sizeof(test_value); i++) {
        std::cout << "0x" << std::hex << static_cast<int>(bytes[i]) << " ";
    }
    std::cout << std::dec << std::endl;
    
    std::cout << "Swapped: 0x" << std::hex << Exs_Swap32(test_value) << std::dec << std::endl;
    std::cout << "=========================" << std::endl;
}

void Exs_Endian::Exs_TestEndianConversion() {
    std::cout << "=== Exs Endian Test ===" << std::endl;
    
    uint16_t test16 = 0x1234;
    uint32_t test32 = 0x12345678;
    uint64_t test64 = 0x123456789ABCDEF0ULL;
    
    std::cout << "Original 16-bit: 0x" << std::hex << test16 << std::dec << std::endl;
    std::cout << "Swapped 16-bit: 0x" << std::hex << Exs_Swap16(test16) << std::dec << std::endl;
    
    std::cout << "Original 32-bit: 0x" << std::hex << test32 << std::dec << std::endl;
    std::cout << "Swapped 32-bit: 0x" << std::hex << Exs_Swap32(test32) << std::dec << std::endl;
    
    std::cout << "Little Endian 32-bit: 0x" << std::hex 
              << Exs_ToLittleEndian32(test32) << std::dec << std::endl;
    std::cout << "Big Endian 32-bit: 0x" << std::hex 
              << Exs_ToBigEndian32(test32) << std::dec << std::endl;
    
    std::cout << "Network order (HTONL): 0x" << std::hex 
              << Exs_HTONL(test32) << std::dec << std::endl;
    std::cout << "Host order (NTOHL): 0x" << std::hex 
              << Exs_NTOHL(Exs_HTONL(test32)) << std::dec << std::endl;
    
    std::cout << "=========================" << std::endl;
}

} // namespace Platform
} // namespace Exs
