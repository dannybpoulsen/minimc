#ifndef _VM_CONRETE_VALUE__
#define _VM_CONRETE_VALUE__

#include <iosfwd>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      class BoolValue {
      public:
        friend struct Caster;
        BoolValue(bool val = false) : val(val) {}
        BoolValue BoolNegate() const { return BoolValue(!val); }
        MiniMC::Hash::hash_t hash() const {
          return val;
        }

        auto getValue() const { return val; }
	
      protected:
        bool val;
      };

      inline std::ostream& operator<<(std::ostream& os, const BoolValue& v) { return os << v.getValue(); }

      template <typename T>
      requires std::is_integral_v<T>
      struct TValue;

      class PointerValue {
      public:
        friend struct Caster;
        PointerValue(MiniMC::pointer_t val) : val(val) {}

        auto getValue() const { return val; }

        MiniMC::Hash::hash_t hash() const {
          return std::bit_cast<MiniMC::Hash::hash_t>(val);
        }

      protected:
        MiniMC::pointer_t val;
      };

      inline std::ostream& operator<<(std::ostream& os, const PointerValue& v) { return os << v.getValue(); }

      template <typename T>
      requires std::is_integral_v<T>
      struct TValue {
        using underlying_type = T;
        TValue(T val) : value(val) {}

        MiniMC::Hash::hash_t hash() const {
          return value;
        }

        static constexpr std::size_t intbitsize() { return sizeof(T) * 8; }

        auto getValue() const { return value; }

      private:
        T value;
      };

      struct AggregateValue {
        AggregateValue(const MiniMC::Util::Array& array) : val(array) {}
        AggregateValue(const MiniMC::Util::Array&& array) : val(std::move(array)) {}

        MiniMC::Hash::hash_t hash() const { return val.hash(0); }
        auto getValue() const { return val; }

      private:
        MiniMC::Util::Array val;
      };

      inline std::ostream& operator<<(std::ostream& os, const AggregateValue&) { return os << "Aggre"; }

      template <class T>
      inline std::ostream& operator<<(std::ostream& os, const TValue<T>& v) { return os << v.getValue(); }

      using ConcreteVMVal = MiniMC::VMT::GenericVal<TValue<MiniMC::uint8_t>,
                                                    TValue<MiniMC::uint16_t>,
                                                    TValue<MiniMC::uint32_t>,
                                                    TValue<MiniMC::uint64_t>,
                                                    PointerValue,
                                                    BoolValue,
                                                    AggregateValue>;

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC

#endif
