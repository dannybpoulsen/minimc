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

  enum class PointerType {
    Stack = 'D',
    Heap = 'H',
    Location = 'L',
    Function = 'F'
  };

  using seg_t = BV8;//decltype(pointer_t::segment);
  using base_t = BV16;//decltype(pointer_t::base);
  using func_t = base_t;
  using proc_t = base_t;
  using offset_t = BV32; // decltype(pointer_t::offset);

  

  
  template <typename seg_t = BV16,
            typename base_t = BV16,
            typename offset_t = BV32,
            typename Ptrbv = BV64>
  struct __attribute__((packed)) pointer_struct {
    //Used for identifying if the pointer is a
    // data pointer
    // location pointer
    // function pointer
    seg_t segment{0};
    //base pointer
    //for function and location pointers base is the function_id
    base_t base{0};
    // offset into base_object
    //for function pointer offset must be zero
    //for location pointer offset is the location inside the function jumped to
    offset_t offset{0};
    using PtrBV = Ptrbv;  
    pointer_struct add (offset_t off) const  {
      return pointer_struct {.segment = segment,.base = base,.offset = offset+off};
    }

    bool is_null () const {return base == 0 && segment == 0 && offset == 0;}

    static auto makeFunctionPointer (MiniMC::func_t f) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerType::Function);
      ptr.base = f;
      ptr.offset = 0;
      return ptr;
    }

    static auto makeLocationPointer (MiniMC::func_t f,offset_t o) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerType::Location);
      ptr.base = f;
      ptr.offset = o;
      return ptr;
    }

    static auto makeHeapPointer (MiniMC::base_t f,offset_t o) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerType::Heap);
      ptr.base = f;
      ptr.offset = o;
      return ptr;
    }

    static auto makeStackPointer (MiniMC::base_t f,offset_t o) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerType::Stack);
      ptr.base = f;
      ptr.offset = o;
      return ptr;
    }
    
  };

  
  
  
  using pointer64_t = pointer_struct<BV16,BV16,BV32,BV64>;
  using pointer32_t = pointer_struct<BV8,BV8,BV16,BV32>;

#ifndef MINIMC32
  using pointer_t = pointer64_t;
#else
  using pointer_t = pointer64_t;
#endif
  

  

  inline bool operator==(const pointer_t& l, const pointer_t& r) {
    return l.segment == r.segment &&
           l.base == r.base &&
           l.offset == r.offset;
  }

  
  template<typename T>
  struct is_pointer : public std::false_type {};

  template<>
  struct is_pointer<pointer32_t> : public std::true_type {};

    template<>
  struct is_pointer<pointer64_t> : public std::true_type {};

  
  template<typename T>
  constexpr bool is_pointer_v = is_pointer<T>::value;

  template<class P>
  bool is_null(const P& t) requires (is_pointer_v<P>) {
    return t.segment == 0 &&
           t.base == 0 &&
           t.offset == 0;
  }

  inline func_t getFunctionId(const pointer_t& p) {
    return p.base;
  }
  
  inline base_t getBase(const pointer_t& p) {
    return p.base;
  }
  
  inline offset_t getOffset(const pointer_t& p) {
    return p.offset;
  }
  
  template <class T, class P>
  T& operator<<(T& os, const P& p)  requires (is_pointer_v<P>) {
    if (is_null(p)) {
      return os << std::string("nullptr", 7);
    }
    return os << static_cast<BV8> (p.segment) << "(" << static_cast<int64_t>(p.base) << "+" << p.offset << ")";
  }
  
  
} // namespace MiniMC

#endif
