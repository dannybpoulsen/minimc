#ifndef _TYPES__
#define _TYPES__

#include <cstdint>

namespace MiniMC {

  enum class InternalTypes {
	      uint8,
	      uint16,
	      uint32,
	      uint64,
	      int8,
	      int16,
	      int32,
	      int64,
  };

  template<InternalTypes>
  struct TypeD {
    using type = std::int8_t;
  };

  template<>
  struct TypeD<InternalTypes::uint8> {
    using type = std::uint8_t;
  };

  template<>
  struct TypeD<InternalTypes::uint16> {
    using type = std::uint16_t;
  };

  template<>
  struct TypeD<InternalTypes::uint32> {
    using type = std::uint32_t;
  };

  template<>
  struct TypeD<InternalTypes::uint64> {
    using type = std::uint64_t;
  };

  template<>
  struct TypeD<InternalTypes::int64> {
    using type = std::int64_t;
  };

  template<>
  struct TypeD<InternalTypes::int32> {
    using type = std::int32_t;
  };

  template<>
  struct TypeD<InternalTypes::int16> {
    using type = std::int16_t;
  };

  template<>
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

  template<class From, class To>
  To bit_cast (const From& f) {
    static_assert(sizeof(From) == sizeof(To));
    return reinterpret_cast<const To&> (f);
  }
  
}


#endif
