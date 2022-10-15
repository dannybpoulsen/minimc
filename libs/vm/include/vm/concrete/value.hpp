#ifndef _VM_CONRETE_VALUE__
#define _VM_CONRETE_VALUE__

#include <iosfwd>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      class BoolValue {
      public:
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
      requires std::is_integral_v<T> || MiniMC::is_pointer_v<T>
      struct TValue {
        using underlying_type = T;
        TValue(T val) : value(val) {}

        MiniMC::Hash::hash_t hash() const {
	  if constexpr (std::is_integral_v<T>) 
	    return value;
	  else if constexpr (MiniMC::is_pointer_v<T>)  {
	    return std::bit_cast<typename T::PtrBV>(value);  
	  }
        }

        static constexpr std::size_t intbitsize() { return sizeof(T) * 8; }

        auto getValue() const { return value; }

      private:
        T value;
      };

      using PointerValue = TValue<MiniMC::pointer64_t>;
      
      struct AggregateValue {
        AggregateValue(const MiniMC::Util::Array& array) : val(array) {}
        AggregateValue(const MiniMC::Util::Array&& array) : val(std::move(array)) {}

        MiniMC::Hash::hash_t hash() const { return val.hash(); }
        auto getValue() const { return val; }

      private:
        MiniMC::Util::Array val;
      };

      inline std::ostream& operator<<(std::ostream& os, const AggregateValue& aggr) { return os << aggr.getValue (); }

      template <class T>
      inline std::ostream& operator<<(std::ostream& os, const TValue<T>& v) {
	if constexpr (!std::is_same_v<MiniMC::BV8,T>) 
	  return os << v.getValue();
	else {
	  return os << (0xFF & static_cast<MiniMC::BV32> (v.getValue()));
	}
      }

      using ConcreteVMVal = MiniMC::VMT::GenericVal<TValue<MiniMC::BV8>,
                                                    TValue<MiniMC::BV16>,
                                                    TValue<MiniMC::BV32>,
                                                    TValue<MiniMC::BV64>,
						    PointerValue,
						    TValue<MiniMC::pointer32_t>,
                                                    BoolValue,
                                                    AggregateValue>;
      
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC

#endif
