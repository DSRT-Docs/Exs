#pragma once
#include "BasicTypes.h"
#include <limits>
#include <cmath>

namespace Exs {
namespace Types {

class Exs_FloatTypes {
public:
    // Type traits for floats
    template<typename T>
    static bool Exs_IsFloat();
    
    template<typename T>
    static bool Exs_IsFloat32();
    
    template<typename T>
    static bool Exs_IsFloat64();
    
    // Float properties
    template<typename T>
    static T Exs_GetMinValue();
    
    template<typename T>
    static T Exs_GetMaxValue();
    
    template<typename T>
    static T Exs_GetEpsilon();
    
    template<typename T>
    static T Exs_GetInfinity();
    
    template<typename T>
    static T Exs_GetNaN();
    
    template<typename T>
    static int Exs_GetDigits();
    
    template<typename T>
    static int Exs_GetMaxDigits10();
    
    // Float classification
    template<typename T>
    static bool Exs_IsFinite(T value);
    
    template<typename T>
    static bool Exs_IsInfinite(T value);
    
    template<typename T>
    static bool Exs_IsNaN(T value);
    
    template<typename T>
    static bool Exs_IsNormal(T value);
    
    template<typename T>
    static bool Exs_IsSubnormal(T value);
    
    template<typename T>
    static int Exs_FPClassify(T value);
    
    // Float comparisons
    template<typename T>
    static bool Exs_ApproximatelyEqual(T a, T b, T epsilon);
    
    template<typename T>
    static bool Exs_ApproximatelyEqualRelative(T a, T b, T maxRelativeError);
    
    template<typename T>
    static bool Exs_ApproximatelyEqualAbs(T a, T b, T maxAbsoluteError);
    
    // Float operations
    template<typename T>
    static T Exs_Round(T value);
    
    template<typename T>
    static T Exs_Floor(T value);
    
    template<typename T>
    static T Exs_Ceil(T value);
    
    template<typename T>
    static T Exs_Trunc(T value);
    
    template<typename T>
    static T Exs_Fract(T value);
    
    template<typename T>
    static T Exs_Mod(T x, T y);
    
    template<typename T>
    static T Exs_Clamp(T value, T minVal, T maxVal);
    
    template<typename T>
    static T Exs_Saturate(T value);  // Clamp to [0, 1]
    
    template<typename T>
    static T Exs_Lerp(T a, T b, T t);
    
    template<typename T>
    static T Exs_InverseLerp(T a, T b, T value);
    
    // Float utilities
    template<typename T>
    static T Exs_DegreesToRadians(T degrees);
    
    template<typename T>
    static T Exs_RadiansToDegrees(T radians);
    
    template<typename T>
    static bool Exs_IsPowerOfTwo(T value);
    
    template<typename T>
    static T Exs_NextPowerOfTwo(T value);
    
    // Float constants
    static constexpr Exs_BasicTypes::Exs_float32 Exs_Float32_Epsilon = 1.192092896e-07f;
    static constexpr Exs_BasicTypes::Exs_float64 Exs_Float64_Epsilon = 2.2204460492503131e-16;
    
    static constexpr Exs_BasicTypes::Exs_float32 Exs_Float32_PI = 3.14159265358979323846f;
    static constexpr Exs_BasicTypes::Exs_float64 Exs_Float64_PI = 3.14159265358979323846;
    
    static constexpr Exs_BasicTypes::Exs_float32 Exs_Float32_HalfPI = 1.57079632679489661923f;
    static constexpr Exs_BasicTypes::Exs_float64 Exs_Float64_HalfPI = 1.57079632679489661923;
    
    static constexpr Exs_BasicTypes::Exs_float32 Exs_Float32_TwoPI = 6.28318530717958647692f;
    static constexpr Exs_BasicTypes::Exs_float64 Exs_Float64_TwoPI = 6.28318530717958647692;
    
    // Debug functions
    static void Exs_DumpFloatTypes();
    
    template<typename T>
    static void Exs_DumpFloatInfo(T value);
};

} // namespace Types
} // namespace Exs
