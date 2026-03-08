#include "minimc/model/instructions.hpp"
#include "minimc/model/modifications/instrument_loads.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/support/workinglist.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      bool InstrumentLoads::runFunction(const MiniMC::Model::Function_ptr& F) {
	auto frame = F->getFrame ();
	//MiniMC::Model::LocationInfoCreator locc(F->getRegisterDescr ());
	auto& cfg = F->getCFA();
	MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
	auto inserter = wlist.inserter();
	std::for_each(cfg.getEdges().begin(),
                        cfg.getEdges().end(),
		      [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
	auto info = MiniMC::Model::LocationInfo{{MiniMC::Model::Attributes::AssertViolated},F->getRegisterDescr(),F->getFrame()};
	auto eloc = cfg.makeLocation(frame.makeFresh ("Assert"),info);
	eloc->getInfo().getFlags () |= MiniMC::Model::Attributes::AssertViolated;

	throw MiniMC::Support::Exception ("Load Instrumentation not implemented");

	return true;
      }
      
    }
  }
}

