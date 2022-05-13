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
  using proba_t = double;

  using seg_t = BV8;
  using base_t = BV16;
  using proc_t = BV16;
  using offset_t = BV32;
  using func_t = base_t;

  struct __attribute__((packed)) pointer_struct {
    //Used for identifying if the pointer is a
    // data pointer
    // location pointer
    // function pointer
    seg_t segment{0};
    // pure padding
    int8_t zero{0}; //must be zero
    //base pointer
    //for function and location pointers base is the function_id
    base_t base{0};
    // offset into base_object
    //for function pointer offset must be zero
    //for location pointer offset is the location inside the function jumped to
    offset_t offset{0};
  };

  using pointer_t = pointer_struct;

  inline bool is_null(const pointer_t& t) {
    return t.segment == 0 &&
           t.zero == 0 &&
           t.base == 0 &&
           t.offset == 0;
  }

  template <class T>
  T& operator<<(T& os, const pointer_t& p) {
    if (is_null(p)) {
      return os << std::string("nullptr", 7);
    }
    return os << p.segment << ":" << static_cast<int64_t>(p.base) << "+" << p.offset;
  }

  inline bool operator==(const pointer_t& l, const pointer_t& r) {
    return l.segment == r.segment &&
           l.zero == r.zero &&
           l.base == r.base &&
           l.offset == r.offset;
  }

  
  template <class From, class To>
  const To& bit_cast(const From& f) {
    static_assert(sizeof(From) == sizeof(To));
    return reinterpret_cast<const To&>(f);
  }

} // namespace MiniMC

#endif
