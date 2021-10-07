#ifmdef _PREDICATE__
#define _PREDICATE__

#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "util/ssamap.hpp
#include "smt/builder.hpp"
#include "util/smtconstuction.hpp"

namespace MiniMC {
  namespace Util {

    class Predicate {
    public:
      Predicate(MiniMC::Model::Variable_ptr& left,
                MiniMC::Model::Variable_ptr& right) : left(left),
                                                      right(right) {}
      virtual ~Predicate() {}

    public:
      virtual SMTLib::Term_ptr instantiatePredicate(SSAMap& map) = 0;

    protected:
      MiniMC::Model::Variable_ptr left;
      MiniMC::Model::Variable_ptr right;
    };

    template <MiniMC::Model::InstructionCode Inst>
    class PredicateImpl : public Predicate {
    public:
      PredicateImpl(MiniMC::Model::Variable_ptr& left,
                    MiniMC::Model::Variable_ptr& right) : Predicate(left, right) {
        if constexpr (!MiniMC::Model::InstructionData<Inst>::isComparison >) {
          static_assert(false);
        }
      }

      virtual SMTLib::Term_ptr instantiatePredicate(SSAMap& map, SMTLib::TermBuilder& builder) {
        auto leftTerm = buildSMTTerm(map, builder, left.get());
        auto rightTerm = map.lookup(map, builder, right.get());
        builder.makeTerm(convertToSMTOp<Inst>(), {leftTerm, rightTerm});
      }
    }
  };
}
}

#endif
