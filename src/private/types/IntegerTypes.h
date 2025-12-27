#pragma once
#include "BasicTypes.h"

namespace Exs {
namespace Types {

class Exs_IntegerTypes {
public:
    // Type traits for integers
    template<typename T>
    static bool Exs_IsSignedInteger();
    
    template<typename T>
    static bool Exs_IsUnsignedInteger();
    
    template<typename T>
    static bool Exs_Is8BitInteger();
    
    template<typename T>
    static bool Exs_Is16BitInteger();
    
    template<typename T>
    static bool Exs_Is32BitInteger();
    
    template<typename T>
    static bool Exs_Is64BitInteger();
    
    // Integer properties
    template<typename T>
    static T Exs_GetMinValue();
    
    template<typename T>
    static T Exs_GetMaxValue();
    
    template<typename T>
    static size_t Exs_GetBitWidth();
    
    // Integer operations
    template<typename T>
    static T Exs_Abs(T value);
    
    template<typename T>
    static T Exs_Sign(T value);
    
    template<typename T>
    static bool Exs_IsPowerOfTwo(T value);
    
    template<typename T>
    static T Exs_NextPowerOfTwo(T value);
    
    template<typename T>
    static T Exs_PrevPowerOfTwo(T value);
    
    // Integer casting with overflow checking
    template<typename To, typename From>
    static To Exs_SafeCast(From value, bool* overflow = nullptr);
    
    // Integer byte swapping
    template<typename T>
    static T Exs_SwapBytes(T value);
    
    // Bit manipulation
    template<typename T>
    static T Exs_RotateLeft(T value, int shift);
    
    template<typename T>
    static T Exs_RotateRight(T value, int shift);
    
    template<typename T>
    static int Exs_CountLeadingZeros(T value);
    
    template<typename T>
    static int Exs_CountTrailingZeros(T value);
    
    template<typename T>
    static int Exs_CountOnes(T value);
    
    // Debug functions
    static void Exs_DumpIntegerTypes();
    template<typename T>
    static void Exs_DumpIntegerInfo();
};

} // namespace Types
} // namespace Exs
