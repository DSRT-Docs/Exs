#pragma once
#include <cstdint>
#include <cstddef>

namespace Exs {
namespace Types {

class Exs_BasicTypes {
public:
    // Integer types
    using Exs_int8    = int8_t;
    using Exs_int16   = int16_t;
    using Exs_int32   = int32_t;
    using Exs_int64   = int64_t;
    
    using Exs_uint8   = uint8_t;
    using Exs_uint16  = uint16_t;
    using Exs_uint32  = uint32_t;
    using Exs_uint64  = uint64_t;
    
    // Floating point types
    using Exs_float32 = float;
    using Exs_float64 = double;
    
    // Size types
    using Exs_size_t  = std::size_t;
    using Exs_ssize_t = std::ptrdiff_t;
    
    // Pointer types
    using Exs_intptr  = intptr_t;
    using Exs_uintptr = uintptr_t;
    
    // Character types
    using Exs_char8   = char;
    using Exs_char16  = char16_t;
    using Exs_char32  = char32_t;
    using Exs_wchar   = wchar_t;
    
    // Boolean type
    using Exs_bool8   = bool;
    
    // Void type (for completeness)
    using Exs_void    = void;
    
    // Type size constants
    static constexpr size_t Exs_SizeOfInt8    = sizeof(Exs_int8);
    static constexpr size_t Exs_SizeOfInt16   = sizeof(Exs_int16);
    static constexpr size_t Exs_SizeOfInt32   = sizeof(Exs_int32);
    static constexpr size_t Exs_SizeOfInt64   = sizeof(Exs_int64);
    
    static constexpr size_t Exs_SizeOfUint8   = sizeof(Exs_uint8);
    static constexpr size_t Exs_SizeOfUint16  = sizeof(Exs_uint16);
    static constexpr size_t Exs_SizeOfUint32  = sizeof(Exs_uint32);
    static constexpr size_t Exs_SizeOfUint64  = sizeof(Exs_uint64);
    
    static constexpr size_t Exs_SizeOfFloat32 = sizeof(Exs_float32);
    static constexpr size_t Exs_SizeOfFloat64 = sizeof(Exs_float64);
    
    // Type alignment constants
    static constexpr size_t Exs_AlignOfInt8    = alignof(Exs_int8);
    static constexpr size_t Exs_AlignOfInt16   = alignof(Exs_int16);
    static constexpr size_t Exs_AlignOfInt32   = alignof(Exs_int32);
    static constexpr size_t Exs_AlignOfInt64   = alignof(Exs_int64);
    
    // Utility methods
    static const char* Exs_GetTypeNameInt8()    { return "int8"; }
    static const char* Exs_GetTypeNameInt16()   { return "int16"; }
    static const char* Exs_GetTypeNameInt32()   { return "int32"; }
    static const char* Exs_GetTypeNameInt64()   { return "int64"; }
    
    static const char* Exs_GetTypeNameUint8()   { return "uint8"; }
    static const char* Exs_GetTypeNameUint16()  { return "uint16"; }
    static const char* Exs_GetTypeNameUint32()  { return "uint32"; }
    static const char* Exs_GetTypeNameUint64()  { return "uint64"; }
    
    static const char* Exs_GetTypeNameFloat32() { return "float32"; }
    static const char* Exs_GetTypeNameFloat64() { return "float64"; }
    
    static const char* Exs_GetTypeNameBool8()   { return "bool8"; }
    static const char* Exs_GetTypeNameChar8()   { return "char8"; }
    static const char* Exs_GetTypeNameChar16()  { return "char16"; }
    static const char* Exs_GetTypeNameChar32()  { return "char32"; }
    
    // Debug function
    static void Exs_DumpBasicTypes();
};

} // namespace Types
} // namespace Exs
