#ifndef _TYPES__
#define _TYPES__

#include <cstdint>
#include <string>
#include <bit>

namespace MiniMC {

  template<std::size_t bits>
  struct HostType {
  };

  template<>
  struct HostType<8> {
    using Unsigned = std::uint8_t; 
    using Signed = std::int8_t; 
  };

  template<>
  struct HostType<16> {
    using Unsigned = std::uint16_t; 
    using Signed = std::int16_t; 
  };

  template<>
  struct HostType<32> {
    using Unsigned = std::uint32_t; 
    using Signed = std::int32_t; 
  };

  template<>
  struct HostType<64> {
    using Unsigned = std::uint64_t; 
    using Signed = std::int64_t; 
  };

  
 
  enum class TACOperations {
    Add,
    Sub,
    Mul,
    UDiv,
    SDiv,
    Shl,
    LShr,
    AShr,
    And,
    Or,
    Xor,  
  };
  
  enum class CMPOperations {
    SGT,
    UGT,
    SGE,
    UGE,
    SLT,
    ULT,
    SLE,
    ULE,
    EQ,
    NEQ
  };
  

  template<std::size_t i>
  typename HostType<i>::Unsigned FromHostSigned (typename HostType<i>::Signed val) {
    return std::bit_cast<typename HostType<i>::Unsigned> (val);
  }

  using BV8 = HostType<8>::Unsigned;
  using BV16 = HostType<16>::Unsigned;
  using BV32 = HostType<32>::Unsigned;
  using BV64 = HostType<64>::Unsigned;
  
  
  
} // namespace MiniMC

#endif
