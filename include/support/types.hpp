#ifndef _TYPES__
#define _TYPES__

#include <cstdint>
#include <string>

namespace MiniMC {

  enum class InternalTypes {
    uint8,
    uint16,
    uint32,
    uint64,
    int8,
    int16,
    int32,
    int64
  };

  template <InternalTypes>
  struct TypeD {
    using type = std::int8_t;
  };

  template <>
  struct TypeD<InternalTypes::uint8> {
    using type = std::uint8_t;
  };

  template <>
  struct TypeD<InternalTypes::uint16> {
    using type = std::uint16_t;
  };

  template <>
  struct TypeD<InternalTypes::uint32> {
    using type = std::uint32_t;
  };

  template <>
  struct TypeD<InternalTypes::uint64> {
    using type = std::uint64_t;
  };

  template <>
  struct TypeD<InternalTypes::int64> {
    using type = std::int64_t;
  };

  template <>
  struct TypeD<InternalTypes::int32> {
    using type = std::int32_t;
  };

  template <>
  struct TypeD<InternalTypes::int16> {
    using type = std::int16_t;
  };

  template <>
  struct TypeD<InternalTypes::int8> {
    using type = std::int8_t;
  };

  using int8_t = TypeD<InternalTypes::int8>::type;
  using int16_t = TypeD<InternalTypes::int16>::type;
  using int32_t = TypeD<InternalTypes::int32>::type;
  using int64_t = TypeD<InternalTypes::int64>::type;

  using uint8_t = TypeD<InternalTypes::uint8>::type;
  using uint16_t = TypeD<InternalTypes::uint16>::type;
  using uint32_t = TypeD<InternalTypes::uint32>::type;
  using uint64_t = TypeD<InternalTypes::uint64>::type;
  using proba_t = double;

  using seg_t = uint8_t;
  using base_t = uint16_t;
  using proc_t = uint16_t;
  using offset_t = uint32_t;
  using func_t = base_t;

  struct __attribute__((packed)) pointer_struct {
    //Used for identifying if the pointer is a
    // data pointer
    // location pointer
    // function pointer
    seg_t segment;
    // pure padding
    int8_t zero; //must be zero
    //base pointer
    //for function and location pointers base is the function_id
    base_t base;
    // offset into base_object
    //for function pointer offset must be zero
    //for location pointer offset is the location inside the function jumped to
    offset_t offset;
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

  template <typename T>
  struct EquivSigned {
    using type = T;
  };

  template <>
  struct EquivSigned<uint8_t> {
    using type = int8_t;
  };

  template <>
  struct EquivSigned<uint16_t> {
    using type = int16_t;
  };

  template <>
  struct EquivSigned<uint32_t> {
    using type = int32_t;
  };

  template <>
  struct EquivSigned<uint64_t> {
    using type = int64_t;
  };

  template <typename T>
  struct EquivUnsigned {
    using type = T;
  };

  template <>
  struct EquivUnsigned<int8_t> {
    using type = uint8_t;
  };

  template <>
  struct EquivUnsigned<int16_t> {
    using type = uint16_t;
  };

  template <>
  struct EquivUnsigned<int32_t> {
    using type = uint32_t;
  };

  template <>
  struct EquivUnsigned<int64_t> {
    using type = uint64_t;
  };

  template <class From, class To>
  const To& bit_cast(const From& f) {
    static_assert(sizeof(From) == sizeof(To));
    return reinterpret_cast<const To&>(f);
  }

} // namespace MiniMC

#endif
