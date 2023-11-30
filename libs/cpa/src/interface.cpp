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
	if (cfastate.isActive (i))
	  os <<  cfastate.getLocationState ().getLocation (i).getSymbol ().getFullName ();
      }
      os << "]\n";

      auto printVStack = [&os,&state](auto& vstack,auto p ) {
	for (auto& reg : vstack.getRegisters ()) {
	  os << reg->getSymbol().getFullName () << ":\t";
	  
	  for (const auto& datastate : state.dataStates ()) {
	    auto symbval = datastate.getBuilder ().buildValue (p,reg);
	    os << "  " << *datastate.getConcretizer ()->evaluate (*symbval);
	  }
	  os << std::endl;
	}
      };
      
      for (std::size_t p = 0; p < nbProcs; p++) {
	if (cfastate.isActive (p)) {
	  printVStack (cfastate.getLocationState().getLocation(p).getInfo().getRegisters (),p);
	}
      }
      
      
      return os << std::endl;;
    }
    
    std::ostream& StateOutputter::output (const AnalysisState& state, std::ostream& os) {
      auto& cfastate = state.getCFAState ();
      auto nbProcs = cfastate.getLocationState ().nbOfProcesses ();
      os << "[";
      for (std::size_t i = 0; i < nbProcs; i++) {
        if(i != 0){
          os << ",";
        }
	os <<  cfastate.getLocationState ().getLocation (i).getSymbol().getName ();
      }
      os << "]\n";

      auto printVStack = [&os,&state](auto& vstack,auto p ) {
	for (auto& reg : vstack.getRegisters ()) {
	  os << reg->getSymbol().getFullName () << ":\t";
	  
	  for (const auto& datastate : state.dataStates ()) {
	    auto symbval = datastate.getBuilder ().buildValue (p,reg);
	    os << "  " << *datastate.getConcretizer ()->evaluate (*symbval);
	  }
	  os << std::endl;
	}
      };

      
      for (std::size_t p = 0; p < nbProcs; p++) {
	printVStack (prgm.getCPURegs (), p);
	printVStack (cfastate.getLocationState().getLocation(p).getInfo().getRegisters (),p);
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
