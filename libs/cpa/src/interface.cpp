#include "minimc/cpa/interface.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/support/overload.hpp"
#include <ostream>

namespace MiniMC {
  namespace CPA {
    

    std::ostream& operator<<(std::ostream& os, const AnalysisState& state) {
      auto nbProcs = state.getLocationState ().nbOfProcesses ();
      os << "[";
      for (std::size_t i = 0; i < nbProcs; i++) {
        if(i != 0){
          os << ",";
        }
	if (state.getLocationState().isActive (i))
	  os <<  state.getLocationState ().getLocation (i).getSymbol ().getFullName ();
      }
      os << "]\n";

      auto printVStack = [&os,&state](auto& vstack,auto p ) {
	for (auto& reg : vstack.getRegisters ()) {
	  os << reg.getSymbol().getFullName () << ":\t";
	  
	  for (const auto& datastate : state.dataStates ()) {
	    auto symbval = datastate.getBuilder ().buildValue (p,reg);
	    os << "  " << *datastate.getConcretizer ()->evaluate (*symbval);
	  }
	  os << std::endl;
	}
      };
      
      for (std::size_t p = 0; p < nbProcs; p++) {
	if (state.getLocationState().isActive (p)) {
	  printVStack (state.getLocationState().getLocation(p).getInfo().getRegisters (),p);
	}
      }
      
      
      return os << std::endl;;
    }
    
    std::ostream& StateOutputter::output (const AnalysisState& state, std::ostream& os) {
      auto nbProcs = state.getLocationState ().nbOfProcesses ();
      os << "[";
      for (std::size_t i = 0; i < nbProcs; i++) {
        if(i != 0){
          os << ",";
        }
	os <<  state.getLocationState ().getLocation (i).getSymbol().getName ();
      }
      os << "]\n";

      /* auto printVStack = [&os,&state](auto& vstack,auto p ) {
	 for (auto& reg : vstack.getRegisters ()) {
	  os << reg.getSymbol().getFullName () << ":\t";
	  
	  for (const auto& datastate : state.dataStates ()) {
	    auto symbval = datastate.getBuilder ().buildValue (p,reg);
	    os << "  " << *datastate.getConcretizer ()->evaluate (*symbval);
	  }
	  os << std::endl;
	}
	};*/

      auto print = [&os,&state](auto& evals, auto index ) {
	
	for (const auto& datastate : state.dataStates ()) {
	  os << "{\n";
	  auto& builder = datastate.getBuilder ();
	  auto concretizer = datastate.getConcretizer ();
	  for (auto& eval: evals) {
	    auto symbval = builder.buildValue (index,*eval.value);
	    os << eval.symb.getFullName () << " " <<  *concretizer->evaluate (*symbval) << std::endl;
	   
	  }
	  os <<"}\n";
	}
	
      };
      
      struct EvalStruct {
	EvalStruct (MiniMC::Model::Symbol symb, MiniMC::Model::Value_ptr p) : symb(symb),value(p) {}
	MiniMC::Model::Symbol symb;
	MiniMC::Model::Value_ptr value;
      };
      
      for (std::size_t p = 0; p < nbProcs; p++) {
	std::vector<EvalStruct> values;
	for (const auto& p: state.getLocationState().getLocation(p).getInfo().getFrame().local_and_parent_symbols()) { //prgm.getRootFrame().local_symbols ()) {
	  std::visit (MiniMC::Support::Overload {
	   [&values,&p](const MiniMC::Model::Register_wptr&) {values.emplace_back(p,std::make_shared<MiniMC::Model::SymbolicConstant> (p));},
	    [this,&values,&p](const MiniMC::Model::HeapBlock_wptr& w) {
	      auto heap_block = w.lock();
	      auto aggr = MiniMC::Model::AggregateType::get (heap_block->size);
	      auto constant = std::make_shared<MiniMC::Model::SymbolicConstant> (p);
	      
	      values.emplace_back(p,std::make_shared<MiniMC::Model::LoadExpr> (heap_block->heap_register,constant,aggr));
	      
	    },
	     MiniMC::Support::Ignore{}
	    },
	    p.getUserData()
	    );
	}
	print (values,p);
      }
      
      
      return os << std::endl;;
    }
    
    MiniMC::Hash::hash_t AnalysisState::hash() const {
      MiniMC::Hash::Hasher hashing;
      for (auto& state : dataStates ()) {
	hashing << state;
      }
      return hashing;
    }
    
    
    bool AnalysisTransfer::Transfer (const AnalysisState& state, const Transition& trans, AnalysisState& res) {
      
      
      std::vector<State_ptr> datas;
      auto datastate_view = state.dataStates ();
      auto dit = datastate_view.begin();
      auto tit = dataTransfers.begin ();
      for (; tit != dataTransfers.end (); ++tit,++dit) {
	auto res = (*tit)->doTransfer (*dit,trans);
	if (!res)
	  return false;
	datas.push_back (std::move(res));
      }
      res = AnalysisState{std::move(datas)};
      
      
      return true;
    }
    
    
  } // namespace CPA
} // namespace MiniMC
