#include "FloatTypes.h"
#include <iostream>
#include <cfenv>
#include <cmath>

namespace Exs {
namespace Types {

// Template implementations
template<typename T>
bool Exs_FloatTypes::Exs_IsFloat() {
    return std::is_floating_point<T>::value;
}

template<typename T>
bool Exs_FloatTypes::Exs_IsFloat32() {
    return sizeof(T) == 4 && std::is_floating_point<T>::value;
}

template<typename T>
bool Exs_FloatTypes::Exs_IsFloat64() {
    return sizeof(T) == 8 && std::is_floating_point<T>::value;
}

template<typename T>
T Exs_FloatTypes::Exs_GetMinValue() {
    return std::numeric_limits<T>::min();
}

template<typename T>
T Exs_FloatTypes::Exs_GetMaxValue() {
    return std::numeric_limits<T>::max();
}

template<typename T>
T Exs_FloatTypes::Exs_GetEpsilon() {
    return std::numeric_limits<T>::epsilon();
}

template<typename T>
T Exs_FloatTypes::Exs_GetInfinity() {
    return std::numeric_limits<T>::infinity();
}

template<typename T>
T Exs_FloatTypes::Exs_GetNaN() {
    return std::numeric_limits<T>::quiet_NaN();
}

template<typename T>
int Exs_FloatTypes::Exs_GetDigits() {
    return std::numeric_limits<T>::digits;
}

template<typename T>
int Exs_FloatTypes::Exs_GetMaxDigits10() {
    return std::numeric_limits<T>::max_digits10;
}

template<typename T>
bool Exs_FloatTypes::Exs_IsFinite(T value) {
    return std::isfinite(value);
}

template<typename T>
bool Exs_FloatTypes::Exs_IsInfinite(T value) {
    return std::isinf(value);
}

template<typename T>
bool Exs_FloatTypes::Exs_IsNaN(T value) {
    return std::isnan(value);
}

template<typename T>
bool Exs_FloatTypes::Exs_IsNormal(T value) {
    return std::isnormal(value);
}

template<typename T>
bool Exs_FloatTypes::Exs_IsSubnormal(T value) {
    return std::fpclassify(value) == FP_SUBNORMAL;
}

template<typename T>
int Exs_FloatTypes::Exs_FPClassify(T value) {
    return std::fpclassify(value);
}

template<typename T>
bool Exs_FloatTypes::Exs_ApproximatelyEqual(T a, T b, T epsilon) {
    return std::abs(a - b) <= epsilon;
}

template<typename T>
bool Exs_FloatTypes::Exs_ApproximatelyEqualRelative(T a, T b, T maxRelativeError) {
    if (a == b) return true;
    
    T diff = std::abs(a - b);
    T absA = std::abs(a);
    T absB = std::abs(b);
    T largest = (absB > absA) ? absB : absA;
    
    if (diff <= largest * maxRelativeError) {
        return true;
    }
    return false;
}

template<typename T>
bool Exs_FloatTypes::Exs_ApproximatelyEqualAbs(T a, T b, T maxAbsoluteError) {
    return std::abs(a - b) <= maxAbsoluteError;
}

template<typename T>
T Exs_FloatTypes::Exs_Round(T value) {
    return std::round(value);
}

template<typename T>
T Exs_FloatTypes::Exs_Floor(T value) {
    return std::floor(value);
}

template<typename T>
T Exs_FloatTypes::Exs_Ceil(T value) {
    return std::ceil(value);
}

template<typename T>
T Exs_FloatTypes::Exs_Trunc(T value) {
    return std::trunc(value);
}

template<typename T>
T Exs_FloatTypes::Exs_Fract(T value) {
    return value - std::floor(value);
}

template<typename T>
T Exs_FloatTypes::Exs_Mod(T x, T y) {
    return std::fmod(x, y);
}

template<typename T>
T Exs_FloatTypes::Exs_Clamp(T value, T minVal, T maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

template<typename T>
T Exs_FloatTypes::Exs_Saturate(T value) {
    return Exs_Clamp(value, static_cast<T>(0), static_cast<T>(1));
}

template<typename T>
T Exs_FloatTypes::Exs_Lerp(T a, T b, T t) {
    return a + (b - a) * t;
}

template<typename T>
T Exs_FloatTypes::Exs_InverseLerp(T a, T b, T value) {
    if (a == b) return static_cast<T>(0);
    return (value - a) / (b - a);
}

template<typename T>
T Exs_FloatTypes::Exs_DegreesToRadians(T degrees) {
    return degrees * (Exs_Float64_PI / 180.0);
}

template<typename T>
T Exs_FloatTypes::Exs_RadiansToDegrees(T radians) {
    return radians * (180.0 / Exs_Float64_PI);
}

template<typename T>
bool Exs_FloatTypes::Exs_IsPowerOfTwo(T value) {
    if (value <= 0) return false;
    
    int intValue = static_cast<int>(value);
    return (intValue & (intValue - 1)) == 0;
}

template<typename T>
T Exs_FloatTypes::Exs_NextPowerOfTwo(T value) {
    if (value <= 1) return 1;
    
    int intValue = static_cast<int>(value);
    intValue--;
    intValue |= intValue >> 1;
    intValue |= intValue >> 2;
    intValue |= intValue >> 4;
    intValue |= intValue >> 8;
    intValue |= intValue >> 16;
    intValue++;
    
    return static_cast<T>(intValue);
}

// Explicit template instantiations
template bool Exs_FloatTypes::Exs_IsFloat<Exs_BasicTypes::Exs_float32>();
template bool Exs_FloatTypes::Exs_IsFloat<Exs_BasicTypes::Exs_float64>();

template bool Exs_FloatTypes::Exs_IsFloat32<Exs_BasicTypes::Exs_float32>();
template bool Exs_FloatTypes::Exs_IsFloat64<Exs_BasicTypes::Exs_float64>();

template Exs_BasicTypes::Exs_float32 Exs_FloatTypes::Exs_GetMinValue<Exs_BasicTypes::Exs_float32>();
template Exs_BasicTypes::Exs_float64 Exs_FloatTypes::Exs_GetMinValue<Exs_BasicTypes::Exs_float64>();

template Exs_BasicTypes::Exs_float32 Exs_FloatTypes::Exs_GetMaxValue<Exs_BasicTypes::Exs_float32>();
template Exs_BasicTypes::Exs_float64 Exs_FloatTypes::Exs_GetMaxValue<Exs_BasicTypes::Exs_float64>();

template Exs_BasicTypes::Exs_float32 Exs_FloatTypes::Exs_GetEpsilon<Exs_BasicTypes::Exs_float32>();
template Exs_BasicTypes::Exs_float64 Exs_FloatTypes::Exs_GetEpsilon<Exs_BasicTypes::Exs_float64>();

void Exs_FloatTypes::Exs_DumpFloatTypes() {
    std::cout << "=== Exs Float Types ===" << std::endl;
    
    std::cout << "float32:" << std::endl;
    std::cout << "  Min Value: " << Exs_GetMinValue<Exs_BasicTypes::Exs_float32>() << std::endl;
    std::cout << "  Max Value: " << Exs_GetMaxValue<Exs_BasicTypes::Exs_float32>() << std::endl;
    std::cout << "  Epsilon: " << Exs_GetEpsilon<Exs_BasicTypes::Exs_float32>() << std::endl;
    std::cout << "  Digits: " << Exs_GetDigits<Exs_BasicTypes::Exs_float32>() << std::endl;
    std::cout << "  Max Digits 10: " << Exs_GetMaxDigits10<Exs_BasicTypes::Exs_float32>() << std::endl;
    
    std::cout << "float64:" << std::endl;
    std::cout << "  Min Value: " << Exs_GetMinValue<Exs_BasicTypes::Exs_float64>() << std::endl;
    std::cout << "  Max Value: " << Exs_GetMaxValue<Exs_BasicTypes::Exs_float64>() << std::endl;
    std::cout << "  Epsilon: " << Exs_GetEpsilon<Exs_BasicTypes::Exs_float64>() << std::endl;
    std::cout << "  Digits: " << Exs_GetDigits<Exs_BasicTypes::Exs_float64>() << std::endl;
    std::cout << "  Max Digits 10: " << Exs_GetMaxDigits10<Exs_BasicTypes::Exs_float64>() << std::endl;
    
    std::cout << "Constants:" << std::endl;
    std::cout << "  PI: " << Exs_Float64_PI << std::endl;
    std::cout << "  Half PI: " << Exs_Float64_HalfPI << std::endl;
    std::cout << "  Two PI: " << Exs_Float64_TwoPI << std::endl;
    
    std::cout << "=========================" << std::endl;
}

} // namespace Types
} // namespace Exs
