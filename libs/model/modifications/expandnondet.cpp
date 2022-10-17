#include "model/modifications/expandnondet.hpp"
#include "model/instructions.hpp"
#include "support/workinglist.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"


#include <limits>
#include <algorithm>

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      struct NonDetGenerator {
	NonDetGenerator (MiniMC::BV64 min, MiniMC::BV64 max) : cur(min), max(max) {}
	auto get () const {return cur;}
	bool finished () const {return finished_;}
	void increment () {
	  if (cur == max) {
	    finished_ = true;
	  }

	  else
	    ++cur ;
	}	
      private:
	MiniMC::BV64  cur;	
	MiniMC::BV64  max;
	
	bool finished_{false};
      };

      void expandEdge (MiniMC::Model::CFA& cfa, MiniMC::Model::ConstantFactory& cfac, const MiniMC::Model::Edge* edge) {
	if (edge->getInstructions ()) {
	  // Only bother if we have instructions to deal with
	  auto& instr = edge->getInstructions();
	  MiniMC::Model::InstructionStream nstr{instr.isPhi ()};
	  auto prev =  edge->getFrom ();
	  auto goal =  edge->getTo ();
	  
	  for (auto& i : instr) {
	    if (i.getOpcode () != MiniMC::Model::InstructionCode::NonDet) {
	      nstr.addInstruction (i);
	    }

	    else {
	      auto nloc = cfa.makeLocation (prev->getInfo ());
	      auto nnondet = cfa.makeLocation (prev->getInfo ());
	      
	      cfa.makeEdge (prev,nloc)->getInstructions () = nstr;
	      nstr.clear ();
	      MiniMC::BV64 min{0};
	      MiniMC::BV64 max{0};
	      auto assign = i.template getOps<InstructionCode::NonDet> ().res;
	      switch (assign->getType()->getTypeID ()) {
	      case TypeID::I8:
		min = std::numeric_limits<MiniMC::BV8>::min ();
		max = std::numeric_limits<MiniMC::BV8>::max ();
		break;
	      case TypeID::I16:
		min = std::numeric_limits<MiniMC::BV16>::min ();
		max = std::numeric_limits<MiniMC::BV16>::max ();
		break;
	      case TypeID::I32:
		min = std::numeric_limits<MiniMC::BV32>::min ();
		max = std::numeric_limits<MiniMC::BV32>::max ();
		break;
	      case TypeID::I64:
		min = std::numeric_limits<MiniMC::BV64>::min ();
		max = std::numeric_limits<MiniMC::BV64>::max ();
		break;
	      default:
		throw MiniMC::Support::Exception ("Cann't unfold this type");
	      }
	      
	      NonDetGenerator gen {min,max};
	      for (;!gen.finished (); 	gen.increment ()) {
		nstr.addInstruction<InstructionCode::Assign> ({.res = assign,.op1 = cfac.makeIntegerConstant (gen.get(),assign->getType()->getTypeID ())});
		cfa.makeEdge (nloc,nnondet)->getInstructions () = nstr;
		nstr.clear ();
	      }

	      prev = nnondet;
	      
	      
	    }
	  }
	  cfa.makeEdge (prev,goal)->getInstructions () = nstr;
	  cfa.deleteEdge ( edge);
	}
      }
	
      void expandNonDetCFAEdges (MiniMC::Model::CFA& cfa, MiniMC::Model::ConstantFactory& cfac) {
	MiniMC::Support::WorkingList<const MiniMC::Model::Edge*> wlist;
	std::for_each (cfa.getEdges().begin (),cfa.getEdges().end (),[&wlist](auto& e) {wlist.inserter () = e.get ();});
	std::for_each (wlist.begin(), wlist.end (),[&cfa,&cfac](auto& e) {
	  expandEdge (cfa,cfac,e);
	}
	  );
      }

      void expandNonDet (MiniMC::Model::Program& prgm) {
	MiniMC::Support::Messager{}.message (MiniMC::Support::Localiser{"Unfolding non-determinstic values"}.format ());
	for (auto& function : prgm.getFunctions ()) {
	  expandNonDetCFAEdges (function->getCFA (),prgm.getConstantFactory ());
	}
      }
      
    }
  }
}
  
