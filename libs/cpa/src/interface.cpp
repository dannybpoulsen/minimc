#include "cpa/interface.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include <ostream>

namespace MiniMC {
  namespace CPA {
    

    std::ostream& operator<<(std::ostream& os, const AnalysisState& state) {
      auto& cfastate = state.getCFAState ();
      auto nbProcs = cfastate.getLocationState ().nbOfProcesses ();
      os << "[";
      for (std::size_t i = 0; i < nbProcs; i++) {
        if(i != 0){
          os << ",";
        }
	os <<  cfastate.getLocationState ().getLocation (i).getInfo().getName ();
      }
      os << "]\n";

      for (std::size_t p = 0; p < nbProcs; p++) {
	auto& vstack = cfastate.getLocationState().getLocation(p).getInfo().getRegisters ();
	for (auto& reg : vstack.getRegisters ()) {
	  os << reg->getName () << ":\t";
	  
	  for (const auto& datastate : state.dataStates ()) {
	    auto symbval = datastate.getBuilder ().buildValue (p,reg);
	    os << "  " << *datastate.getConcretizer ()->evaluate (*symbval);
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
    
    
    bool AnalysisTransfer::Transfer (const AnalysisState& state, const Transition& trans, AnalysisState& res) {
      
      auto locTrans = locTransfer->doTransfer (state.getCFAState (),trans);
      if (!locTrans)
	return false;
      else {
	std::vector<DataState_ptr> datas;
	auto datastate_view = state.dataStates ();
	auto dit = datastate_view.begin();
	auto tit = dataTransfers.begin ();
	for (; tit != dataTransfers.end (); ++tit,++dit) {
	  auto res = (*tit)->doTransfer (*dit,trans);
	  if (!res)
	    return false;
	  datas.push_back (std::move(res));
	}
	res = AnalysisState{std::move(locTrans),std::move(datas)};
	
      }
      return true;
    }
   
    
  } // namespace CPA
} // namespace MiniMC
