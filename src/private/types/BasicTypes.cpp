#include "BasicTypes.h"
#include <iostream>

namespace Exs {
namespace Types {

void Exs_BasicTypes::Exs_DumpBasicTypes() {
    std::cout << "=== Exs Basic Types ===" << std::endl;
    
    std::cout << "Integer Types:" << std::endl;
    std::cout << "  int8:    " << Exs_SizeOfInt8 << " bytes, align: " << Exs_AlignOfInt8 << std::endl;
    std::cout << "  int16:   " << Exs_SizeOfInt16 << " bytes, align: " << Exs_AlignOfInt16 << std::endl;
    std::cout << "  int32:   " << Exs_SizeOfInt32 << " bytes, align: " << Exs_AlignOfInt32 << std::endl;
    std::cout << "  int64:   " << Exs_SizeOfInt64 << " bytes, align: " << Exs_AlignOfInt64 << std::endl;
    
    std::cout << "Unsigned Integer Types:" << std::endl;
    std::cout << "  uint8:   " << Exs_SizeOfUint8 << " bytes" << std::endl;
    std::cout << "  uint16:  " << Exs_SizeOfUint16 << " bytes" << std::endl;
    std::cout << "  uint32:  " << Exs_SizeOfUint32 << " bytes" << std::endl;
    std::cout << "  uint64:  " << Exs_SizeOfUint64 << " bytes" << std::endl;
    
    std::cout << "Floating Point Types:" << std::endl;
    std::cout << "  float32: " << Exs_SizeOfFloat32 << " bytes" << std::endl;
    std::cout << "  float64: " << Exs_SizeOfFloat64 << " bytes" << std::endl;
    
    std::cout << "Character Types:" << std::endl;
    std::cout << "  char8:   " << sizeof(Exs_char8) << " bytes" << std::endl;
    std::cout << "  char16:  " << sizeof(Exs_char16) << " bytes" << std::endl;
    std::cout << "  char32:  " << sizeof(Exs_char32) << " bytes" << std::endl;
    std::cout << "  wchar:   " << sizeof(Exs_wchar) << " bytes" << std::endl;
    
    std::cout << "Boolean Type:" << std::endl;
    std::cout << "  bool8:   " << sizeof(Exs_bool8) << " bytes" << std::endl;
    
    std::cout << "=========================" << std::endl;
}

} // namespace Types
} // namespace Exs
