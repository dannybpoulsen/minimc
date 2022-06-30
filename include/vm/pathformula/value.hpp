#ifndef _VM_PATHFORMULA_VALUES__
#define _VM_PATHFORMULA_VALUES__

#include "vm/vmt.hpp"
#include <memory>

namespace SMTLib {
  class Term;
  using Term_ptr = std::shared_ptr<Term>;
} // namespace SMTLib

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      enum class ValType {
        I8,
        I16,
        I32,
        I64,
        Bool,
        Pointer,
        Aggregate
      };

      template <ValType t>
      constexpr auto intbitsize_v() {
        if constexpr (t == ValType::I8)
          return 8;
        else if constexpr (t == ValType::I16)
          return 16;
        else if constexpr (t == ValType::I32)
          return 32;
        else if constexpr (t == ValType::I64)
          return 64;
        else if constexpr (t == ValType::Pointer)
          return 64;
        else if constexpr (t == ValType::Bool)
          return 8;
        else
          []<bool b = false>() { static_assert(b); }
        ();
      }

      template <ValType v>
      class Value {
      public:
        Value() = default;

        Value(SMTLib::Term_ptr&& term) requires (v != ValType::Aggregate) : term(std::move(term)),
							      bytesize(intbitsize<v>() / 8) {}

	
        Value(SMTLib::Term_ptr&& term,std::size_t b) requires (v == ValType::Aggregate) : term(std::move(term)),
							      bytesize(b) {}
	
	auto& getTerm() const { return term; }

        std::ostream& output(std::ostream& os) const;

        template <ValType t = v>
        static constexpr std::enable_if_t<t != ValType::Aggregate, std::size_t> intbitsize() { return intbitsize_v<v>(); }

        bool operator==(const Value& vv) const { return term.get() == vv.term.get(); }

	std::size_t size () const {return bytesize;}
	
      private:
        SMTLib::Term_ptr term{nullptr};
	std::size_t bytesize;
      };

      template <ValType v>
      auto& operator<<(std::ostream& o, const Value<v>& val) {
        return val.output(o);
      }

      using AggregateValue = Value<ValType::Aggregate>;
      using I64Value = Value<ValType::I64>;
      using I32Value = Value<ValType::I32>;
      using I16Value = Value<ValType::I16>;
      using I8Value = Value<ValType::I8>;
      using BoolValue = Value<ValType::Bool>;
      using PointerValue = Value<ValType::Pointer>;

      using PathFormulaVMVal = MiniMC::VMT::GenericVal<I8Value,
                                                       I16Value,
                                                       I32Value,
                                                       I64Value,
                                                       PointerValue,
                                                       BoolValue,
                                                       AggregateValue>;

    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC

namespace std {
  template <MiniMC::VMT::Pathformula::ValType T>
  struct hash<MiniMC::VMT::Pathformula::Value<T>> {
    auto operator()(const MiniMC::VMT::Pathformula::Value<T>& t) { return bit_cast<MiniMC::Hash::hash_t>(&t); }
  };

  template <>
  struct hash<MiniMC::VMT::Pathformula::PathFormulaVMVal> {
    auto operator()(const MiniMC::VMT::Pathformula::PathFormulaVMVal& t) { return t.hash(); }
  };
} // namespace std

#endif
