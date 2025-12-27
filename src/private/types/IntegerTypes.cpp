#include "IntegerTypes.h"
#include <iostream>
#include <limits>
#include <bitset>

namespace Exs {
namespace Types {

// Template implementations
template<typename T>
bool Exs_IntegerTypes::Exs_IsSignedInteger() {
    return std::is_signed<T>::value && std::is_integral<T>::value;
}

template<typename T>
bool Exs_IntegerTypes::Exs_IsUnsignedInteger() {
    return std::is_unsigned<T>::value && std::is_integral<T>::value;
}

template<typename T>
bool Exs_IntegerTypes::Exs_Is8BitInteger() {
    return sizeof(T) == 1;
}

template<typename T>
bool Exs_IntegerTypes::Exs_Is16BitInteger() {
    return sizeof(T) == 2;
}

template<typename T>
bool Exs_IntegerTypes::Exs_Is32BitInteger() {
    return sizeof(T) == 4;
}

template<typename T>
bool Exs_IntegerTypes::Exs_Is64BitInteger() {
    return sizeof(T) == 8;
}

template<typename T>
T Exs_IntegerTypes::Exs_GetMinValue() {
    return std::numeric_limits<T>::min();
}

template<typename T>
T Exs_IntegerTypes::Exs_GetMaxValue() {
    return std::numeric_limits<T>::max();
}

template<typename T>
size_t Exs_IntegerTypes::Exs_GetBitWidth() {
    return sizeof(T) * 8;
}

template<typename T>
T Exs_IntegerTypes::Exs_Abs(T value) {
    if (Exs_IsSignedInteger<T>()) {
        return value < 0 ? -value : value;
    }
    return value;
}

template<typename T>
T Exs_IntegerTypes::Exs_Sign(T value) {
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

template<typename T>
bool Exs_IntegerTypes::Exs_IsPowerOfTwo(T value) {
    return value > 0 && (value & (value - 1)) == 0;
}

template<typename T>
T Exs_IntegerTypes::Exs_NextPowerOfTwo(T value) {
    if (value <= 1) return 1;
    
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    if (sizeof(T) > 4) {
        value |= value >> 32;
    }
    value++;
    
    return value;
}

template<typename T>
T Exs_IntegerTypes::Exs_PrevPowerOfTwo(T value) {
    if (value <= 1) return 1;
    
    T result = 1;
    while (result * 2 <= value) {
        result *= 2;
    }
    return result;
}

template<typename T>
T Exs_IntegerTypes::Exs_SwapBytes(T value) {
    T result = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
        result = (result << 8) | ((value >> (i * 8)) & 0xFF);
    }
    return result;
}

template<typename T>
T Exs_IntegerTypes::Exs_RotateLeft(T value, int shift) {
    constexpr size_t bits = sizeof(T) * 8;
    shift = shift % bits;
    return (value << shift) | (value >> (bits - shift));
}

template<typename T>
T Exs_IntegerTypes::Exs_RotateRight(T value, int shift) {
    constexpr size_t bits = sizeof(T) * 8;
    shift = shift % bits;
    return (value >> shift) | (value << (bits - shift));
}

// Explicit template instantiations
template bool Exs_IntegerTypes::Exs_IsSignedInteger<Exs_BasicTypes::Exs_int8>();
template bool Exs_IntegerTypes::Exs_IsSignedInteger<Exs_BasicTypes::Exs_int16>();
template bool Exs_IntegerTypes::Exs_IsSignedInteger<Exs_BasicTypes::Exs_int32>();
template bool Exs_IntegerTypes::Exs_IsSignedInteger<Exs_BasicTypes::Exs_int64>();

template bool Exs_IntegerTypes::Exs_IsUnsignedInteger<Exs_BasicTypes::Exs_uint8>();
template bool Exs_IntegerTypes::Exs_IsUnsignedInteger<Exs_BasicTypes::Exs_uint16>();
template bool Exs_IntegerTypes::Exs_IsUnsignedInteger<Exs_BasicTypes::Exs_uint32>();
template bool Exs_IntegerTypes::Exs_IsUnsignedInteger<Exs_BasicTypes::Exs_uint64>();

template Exs_BasicTypes::Exs_int8 Exs_IntegerTypes::Exs_GetMinValue<Exs_BasicTypes::Exs_int8>();
template Exs_BasicTypes::Exs_int16 Exs_IntegerTypes::Exs_GetMinValue<Exs_BasicTypes::Exs_int16>();
template Exs_BasicTypes::Exs_int32 Exs_IntegerTypes::Exs_GetMinValue<Exs_BasicTypes::Exs_int32>();
template Exs_BasicTypes::Exs_int64 Exs_IntegerTypes::Exs_GetMinValue<Exs_BasicTypes::Exs_int64>();

template Exs_BasicTypes::Exs_int8 Exs_IntegerTypes::Exs_GetMaxValue<Exs_BasicTypes::Exs_int8>();
template Exs_BasicTypes::Exs_int16 Exs_IntegerTypes::Exs_GetMaxValue<Exs_BasicTypes::Exs_int16>();
template Exs_BasicTypes::Exs_int32 Exs_IntegerTypes::Exs_GetMaxValue<Exs_BasicTypes::Exs_int32>();
template Exs_BasicTypes::Exs_int64 Exs_IntegerTypes::Exs_GetMaxValue<Exs_BasicTypes::Exs_int64>();

void Exs_IntegerTypes::Exs_DumpIntegerTypes() {
    std::cout << "=== Exs Integer Types ===" << std::endl;
    
    std::cout << "Signed Integers:" << std::endl;
    std::cout << "  int8:  min=" << (int)Exs_GetMinValue<Exs_BasicTypes::Exs_int8>() 
              << ", max=" << (int)Exs_GetMaxValue<Exs_BasicTypes::Exs_int8>() << std::endl;
    std::cout << "  int16: min=" << Exs_GetMinValue<Exs_BasicTypes::Exs_int16>() 
              << ", max=" << Exs_GetMaxValue<Exs_BasicTypes::Exs_int16>() << std::endl;
    std::cout << "  int32: min=" << Exs_GetMinValue<Exs_BasicTypes::Exs_int32>() 
              << ", max=" << Exs_GetMaxValue<Exs_BasicTypes::Exs_int32>() << std::endl;
    std::cout << "  int64: min=" << Exs_GetMinValue<Exs_BasicTypes::Exs_int64>() 
              << ", max=" << Exs_GetMaxValue<Exs_BasicTypes::Exs_int64>() << std::endl;
    
    std::cout << "Unsigned Integers:" << std::endl;
    std::cout << "  uint8:  min=" << (int)Exs_GetMinValue<Exs_BasicTypes::Exs_uint8>() 
              << ", max=" << (int)Exs_GetMaxValue<Exs_BasicTypes::Exs_uint8>() << std::endl;
    std::cout << "  uint16: min=" << Exs_GetMinValue<Exs_BasicTypes::Exs_uint16>() 
              << ", max=" << Exs_GetMaxValue<Exs_BasicTypes::Exs_uint16>() << std::endl;
    std::cout << "  uint32: min=" << Exs_GetMinValue<Exs_BasicTypes::Exs_uint32>() 
              << ", max=" << Exs_GetMaxValue<Exs_BasicTypes::Exs_uint32>() << std::endl;
    std::cout << "  uint64: min=" << Exs_GetMinValue<Exs_BasicTypes::Exs_uint64>() 
              << ", max=" << Exs_GetMaxValue<Exs_BasicTypes::Exs_uint64>() << std::endl;
    
    std::cout << "=========================" << std::endl;
}

} // namespace Types
} // namespace Exs
