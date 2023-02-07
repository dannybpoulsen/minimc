
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
      
      struct Transferer : public MiniMC::CPA::Transferer {
	Transferer (const MiniMC::Model::Program& prgm) : engine(MiniMC::VMT::Concrete::ConcreteEngine::OperationsT{},MiniMC::VMT::Concrete::ConcreteEngine::CasterT{},prgm) {}
        MiniMC::CPA::CommonState_ptr doTransfer(const MiniMC::CPA::CommonState& s, const MiniMC::Model::Edge& e, proc_id id);
      private:
	MiniMC::VMT::Concrete::ConcreteEngine engine;
      };
      
      
      
      struct CPA : public ICPA {
	//virtual StateQuery_ptr makeQuery() const {return std::make_shared<StateQuery> ();}
	CommonState_ptr makeInitialState(const InitialiseDescr&) override;
	virtual Transferer_ptr makeTransfer(const MiniMC::Model::Program& prgm ) const {return std::make_shared<Transferer> (prgm);}
	
      };
    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC

#endif
