#ifndef _VM_PATHFORMULA_VALUES__
#define _VM_PATHFORMULA_VALUES__

#include "minimc/vm/vmt.hpp"
#include <memory>

namespace SMTLib {
  class Term;
  using Term_ptr = std::shared_ptr<Term>;
  class Solver;
  } // namespace SMTLib

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      
      template <typename v>
      class TValue {
      public:
        TValue() = default;
	
        TValue(SMTLib::Term_ptr&& term) requires (std::is_integral_v<v> ||
						 MiniMC::Model::is_pointer_v<v>
						 ) :
						     term(std::move(term)),
						     bytesize(intbitsize<v>() / 8) {}

	
        TValue(SMTLib::Term_ptr&& term,std::size_t b) requires (
							       !std::is_integral_v<v> &&
							       !MiniMC::Model::is_pointer_v<v>
							       ) : term(std::move(term)),
								   bytesize(b) {}
		
	auto& getTerm() const { return term; }

        std::ostream& output(std::ostream& os) const;
	
        template <typename t = v>
        static constexpr std::size_t intbitsize() requires (std::is_integral_v<v> || MiniMC::Model::is_pointer_v<v>) { return sizeof(v)*8; }

        bool operator==(const TValue& vv) const { return term.get() == vv.term.get(); }

	std::size_t size () const {return bytesize;}

	v interpretValue (const SMTLib::Solver&) const;

	using underlying_type = v;
	
      private:
        SMTLib::Term_ptr term{nullptr};
	std::size_t bytesize;
      };

      template <typename v>
      auto& operator<<(std::ostream& o, const TValue<v>& val) {
        return val.output(o);
      }

      using AggregateValue = TValue<MiniMC::Util::Array>;
      using I64Value = TValue<MiniMC::BV64>;
      using I32Value = TValue<MiniMC::BV32>;
      using I16Value = TValue<MiniMC::BV16>;
      using I8Value = TValue<MiniMC::BV8>;
      using BoolValue = TValue<bool>;
      using PointerValue = TValue<MiniMC::Model::pointer64_t>;
      using Pointer32Value = TValue<MiniMC::Model::pointer32_t>;
      
      using Value = MiniMC::VMT::GenericVal<I8Value,
                                                       I16Value,
                                                       I32Value,
                                                       I64Value,
						       PointerValue,
						       Pointer32Value,
						       BoolValue,
                                                       AggregateValue>;

    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC

namespace std {
  template <typename T>
  struct hash<MiniMC::VMT::Pathformula::TValue<T>> {
    auto operator()(const MiniMC::VMT::Pathformula::TValue<T>& t) { return bit_cast<MiniMC::Hash::hash_t>(&t); }
  };

  template <>
  struct hash<MiniMC::VMT::Pathformula::Value> {
    auto operator()(const MiniMC::VMT::Pathformula::Value& t) { return t.hash(); }
  };
} // namespace std

#endif
