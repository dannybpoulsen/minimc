
#ifndef _CPA_CONCRETE_INTERFACE__
#define _CPA_CONCRETE_INTERFACE__

#include "model/checkers/HasInstruction.hpp"
#include <memory>
#include <ostream>

#include "cpa/interface.hpp"
#include "cpa/state.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "vm/concrete/concrete.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include <unordered_map>

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      struct StateQuery : public MiniMC::CPA::StateQuery {

        MiniMC::CPA::State_ptr makeInitialState(const MiniMC::Model::Program&);

        size_t nbOfProcesses(const MiniMC::CPA::State_ptr&) { return 0; }

        MiniMC::Model::Location_ptr getLocation(const MiniMC::CPA::State_ptr&, proc_id) { return nullptr; }
      };

      struct Transferer : public MiniMC::CPA::Transferer {

        MiniMC::CPA::State_ptr doTransfer(const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id);
      private:
	MiniMC::VMT::Concrete::ConcreteEngine engine{MiniMC::VMT::Concrete::Operations{},MiniMC::VMT::Concrete::Caster{}};
      };
      
      struct Joiner : public MiniMC::CPA::Joiner {
        MiniMC::CPA::State_ptr doJoin(const MiniMC::CPA::State_ptr&, const MiniMC::CPA::State_ptr&) { return nullptr; }
        bool covers(const MiniMC::CPA::State_ptr& l, const MiniMC::CPA::State_ptr& r) {
          return l->hash() == r->hash();
        }
      };

      struct PrevalidateSetup : public MiniMC::CPA::PrevalidateSetup {
        bool validate(const MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {
          return MiniMC::Model::Checkers::HasNoInstruction<MiniMC::Model::InstructionCode::Call>{mess, "This CPA does not support '%1%' instructions."}.run(prgm);
        }
      };

      using CPA = CPADef<
          StateQuery,
          Transferer,
          Joiner,
          MiniMC::CPA::Storer,
          MiniMC::CPA::PrevalidateSetup>;

      /*struct CPADef {
	using Query = StateQuery; 
	using Transfer = Transferer;
	using Join = Joiner; 
	using Storage = MiniMC::CPA::Storer<Join>; 
	using PreValidate = PrevalidateSetup;  
      };*/
    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC

#endif
