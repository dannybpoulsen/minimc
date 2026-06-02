#include "minimc/cpa/interface.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace CPA {
    

 
    MiniMC::IO::ostream& CPAConcreteStateOutputter::output (const State& state, MiniMC::IO::ostream& os) {
      auto nbProcs = state.getLocationState ().nbOfProcesses ();
      os << "[";
      for (std::size_t i = 0; i < nbProcs; i++) {
        if(i != 0){
          os << ",";
        }
	os <<  state.getLocationState ().getLocation (i).getSymbol().getFullName () << " # ";
	os <<  state.getLocationState ().getLocation (i).getInfo().getSource();
      }
      os << "]\n";
      

      
      auto print = [&os,&state,this](auto& evals, auto index ) {
	
	os << "{\n";
	auto& builder = state.getBuilder ();
	auto concretizer = state.getConcretizer (options);
	for (auto& eval: evals) {
	  auto symbval = builder.buildValue (index,*eval.value);
	  os << eval.symb.getFullName () << " " <<  *concretizer->evaluate (*symbval) << "\n";
	  
	}
	os <<"}\n";
	
	
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
      
      
      return os << "\n";;
    }
    

       MiniMC::IO::ostream& CPAStateOutputter::output (const State& state, MiniMC::IO::ostream& os) {
      auto nbProcs = state.getLocationState ().nbOfProcesses ();
      os << "[";
      for (std::size_t i = 0; i < nbProcs; i++) {
        if(i != 0){
          os << ",";
        }
	os <<  state.getLocationState ().getLocation (i).getSymbol().getFullName () << " # ";
	os <<  state.getLocationState ().getLocation (i).getInfo().getSource();
      }
      os << "]\n";
      

      
      auto print = [&os,&state](auto& evals, auto index ) {
	
	os << "{\n";
	auto& builder = state.getBuilder ();
	for (auto& eval: evals) {
	  auto symbval = builder.buildValue (index,*eval.value);
	  MiniMC::IO::str_ostream stringstream;
	  stringstream << *symbval;
	  auto str = stringstream.str();
	  os << eval.symb.getFullName () << " " <<  ((str.length() <=20) ? str : std::string{"TOO LARGE"}) << "\n";
	  
	}
	os <<"}\n";
	
	
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
      
      
      return os << "\n";;
    }
    

    
    
  } // namespace CPA
} // namespace MiniMC
