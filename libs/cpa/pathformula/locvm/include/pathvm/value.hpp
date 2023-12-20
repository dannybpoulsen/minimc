#ifndef _VM_PATHFORMULA_VALUES__
#define _VM_PATHFORMULA_VALUES__

#include "vm/vmt.hpp"
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
      class Value {
      public:
        Value() = default;
	
        Value(SMTLib::Term_ptr&& term) requires (std::is_integral_v<v> ||
						 MiniMC::Model::is_pointer_v<v>
						 ) :
						     term(std::move(term)),
						     bytesize(intbitsize<v>() / 8) {}

	
        Value(SMTLib::Term_ptr&& term,std::size_t b) requires (
							       !std::is_integral_v<v> &&
							       !MiniMC::Model::is_pointer_v<v>
							       ) : term(std::move(term)),
								   bytesize(b) {}
		
	auto& getTerm() const { return term; }

        std::ostream& output(std::ostream& os) const;
	
        template <typename t = v>
        static constexpr std::size_t intbitsize() requires (std::is_integral_v<v> || MiniMC::Model::is_pointer_v<v>) { return sizeof(v)*8; }

        bool operator==(const Value& vv) const { return term.get() == vv.term.get(); }

	std::size_t size () const {return bytesize;}

	v interpretValue (const SMTLib::Solver&) const;

	using underlying_type = v;
	
      private:
        SMTLib::Term_ptr term{nullptr};
	std::size_t bytesize;
      };

      template <typename v>
      auto& operator<<(std::ostream& o, const Value<v>& val) {
        return val.output(o);
      }

      using AggregateValue = Value<MiniMC::Util::Array>;
      using I64Value = Value<MiniMC::BV64>;
      using I32Value = Value<MiniMC::BV32>;
      using I16Value = Value<MiniMC::BV16>;
      using I8Value = Value<MiniMC::BV8>;
      using BoolValue = Value<bool>;
      using PointerValue = Value<MiniMC::Model::pointer64_t>;
      using Pointer32Value = Value<MiniMC::Model::pointer32_t>;
      
      using PathFormulaVMVal = MiniMC::VMT::GenericVal<I8Value,
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
  struct hash<MiniMC::VMT::Pathformula::Value<T>> {
    auto operator()(const MiniMC::VMT::Pathformula::Value<T>& t) { return bit_cast<MiniMC::Hash::hash_t>(&t); }
  };

  template <>
  struct hash<MiniMC::VMT::Pathformula::PathFormulaVMVal> {
    auto operator()(const MiniMC::VMT::Pathformula::PathFormulaVMVal& t) { return t.hash(); }
  };
} // namespace std

#endif
