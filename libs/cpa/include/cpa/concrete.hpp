
#ifndef _CPA_CONCRETE_INTERFACE__
#define _CPA_CONCRETE_INTERFACE__

#include <memory>
#include <ostream>

#include "cpa/interface.hpp"
#include "cpa/state.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include <unordered_map>



namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      
      struct Transferer : public MiniMC::CPA::TTransfer<DataState> {
	Transferer (const MiniMC::Model::Program& prgm);
	~Transferer ();
        MiniMC::CPA::DataState_ptr doTransfer(const MiniMC::CPA::DataState& s, const MiniMC::CPA::Transition&) override;
      private:
	struct Internal;
	
	std::unique_ptr<Internal> _internal;
      };
      
      
      
      struct CPA : public ICPA<DataState> {
	//virtual StateQuery_ptr makeQuery() const {return std::make_shared<StateQuery> ();}
	State_ptr<DataState> makeInitialState(const InitialiseDescr&) override;
	virtual TTransferer_ptr<DataState> makeTransfer(const MiniMC::Model::Program& prgm ) const {return std::make_shared<Transferer> (prgm);}
	
      };
    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC

#endif
