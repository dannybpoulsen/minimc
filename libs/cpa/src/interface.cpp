#include "cpa/interface.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include <ostream>

namespace MiniMC {
  namespace CPA {
    

    std::ostream& operator<<(std::ostream& os, const AnalysisState& state) {
      auto& cfastate = state.getCFAState ();
      auto nbProcs = cfastate->getLocationState ().nbOfProcesses ();
      os << "[";
      for (std::size_t i = 0; i < nbProcs; i++) {
        if(i != 0){
          os << ",";
        }
	os <<  cfastate->getLocationState ().getLocation (i).getInfo().getName ();
      }
      os << "]\n";

      auto nbDataStates = state.nbDataStates ();
      for (std::size_t p = 0; p < nbProcs; p++) {
	auto& vstack = cfastate->getLocationState().getLocation(p).getInfo().getRegisters ();
	for (auto& reg : vstack.getRegisters ()) {
	  os << reg->getName () << ":\t";

	  for (std::size_t dstate = 0; dstate < nbDataStates; dstate++) {
	    auto& datastate = state.getDataState (dstate);
	    auto symbval = datastate->getBuilder ().buildValue (p,reg);
	    os << "  " << *datastate->getConcretizer ()->evaluate (*symbval);
	  }
	  os << std::endl;
	}
      }
      
      
      return os << std::endl;;
    }
    
    MiniMC::Hash::hash_t AnalysisState::hash() const {
      MiniMC::Hash::Hasher hashing;
      hashing << *cfastate;
      for (auto& state : datastates) {
	hashing << *state;
      }
      return hashing;
    }
    
    
    bool AnalysisTransfer::Transfer (const AnalysisState& state, const MiniMC::Model::Edge& e,  proc_id proc,AnalysisState& res) {
      auto locTrans = std::static_pointer_cast<const CFAState> (locTransfer->doTransfer (*state.getCFAState (),e,proc));
      if (!locTrans)
	return false;
      else {
	assert(state.nbDataStates () == dataTransfers.size ());
	std::size_t i = 0;
	std::vector<DataState_ptr> datas;
	for (auto tit = dataTransfers.begin (); tit != dataTransfers.end (); ++tit,++i) {
	  auto res = (*tit)->doTransfer (*state.getDataState (i),e,proc);
	  if (!res)
	    return false;
	  datas.push_back (std::move(std::static_pointer_cast<const DataState> (res)));
	}
	res = AnalysisState{std::move(locTrans),std::move(datas)};
	
      }
      return true;
    }
   
    
  } // namespace CPA
} // namespace MiniMC
