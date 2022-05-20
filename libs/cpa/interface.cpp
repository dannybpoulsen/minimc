#include "cpa/interface.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include <ostream>

namespace MiniMC {
  namespace CPA {
    std::ostream& operator<<(std::ostream& os, const CommonState& state) {
      
      return os;
    }

    MiniMC::Hash::hash_t AnalysisState::hash() const {
      MiniMC::Hash::Hasher hashing;
      hashing << *cfastate;
      for (auto& state : datastates) {
	hashing << *state;
      }
      return hashing;
    }
    
    
    bool AnalysisTransfer::Transfer (const AnalysisState& state, const MiniMC::Model::Edge_ptr& e,  proc_id proc,AnalysisState& res) {
      auto locTrans = std::static_pointer_cast<CFAState> (locTransfer->doTransfer (state.getCFAState (),e,proc));
      if (!locTrans)
	return false;
      else {
	assert(state.nbDataStates () == dataTransfers.size ());
	std::size_t i = 0;
	std::vector<DataState_ptr> datas;
	for (auto tit = dataTransfers.begin (); tit != dataTransfers.end (); ++tit,++i) {
	  auto res = (*tit)->doTransfer (state.getDataState (i),e,proc);
	  if (!res)
	    return false;
	  datas.push_back (std::move(std::static_pointer_cast<DataState> (res)));
	}
	res = AnalysisState{std::move(locTrans),std::move(datas)};
	
      }
      return true;
    }
   
    
  } // namespace CPA
} // namespace MiniMC
