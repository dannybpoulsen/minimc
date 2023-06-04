
#include "model/cfg.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/helpers.hpp"
#include "support/exceptions.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"

#include <functional>

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      void inlineCallEdgeToFunction(const MiniMC::Model::Program& prgm, const MiniMC::Model::Function_ptr& func, const MiniMC::Model::Edge_ptr& edge, MiniMC::Model::LocationInfoCreator& locinfoc,  std::function<void(MiniMC::Model::Edge_ptr)> newCall, MiniMC::Model::Frame cframe) {
        
        auto from_loc = edge->getFrom();
        auto to_loc = edge->getTo();
        auto& instrs = edge->getInstructions();
        assert(instrs.last().getOpcode() == MiniMC::Model::InstructionCode::Call);
	auto call_content = instrs.last ().getOps<MiniMC::Model::InstructionCode::Call> ();
	auto constant = std::static_pointer_cast<MiniMC::Model::Pointer>(call_content.function);
        MiniMC::pointer_t loadPtr =  constant->getValue (); 
	auto cfunc = prgm.getFunction(MiniMC::getFunctionId(loadPtr));
	auto frame = cframe.create (cfunc->getSymbol ().getName ());
	MiniMC::Model::SymbolTable<MiniMC::Model::Value_ptr> valmap;
        auto copyVar = [&](MiniMC::Model::RegisterDescr& stack) {
          for (auto& v : stack.getRegisters()) {
            valmap.insert(std::make_pair(v->getSymbol (), func->getRegisterDescr().addRegister(frame.makeSymbol (v->getSymbol ().getName ()), v->getType())));
          }
        };

        copyVar(cfunc->getRegisterDescr());

	MiniMC::Model::SymbolTable<MiniMC::Model::Location_ptr> locmap;
        MiniMC::Support::WorkingList<Edge_ptr> wlist;
	
        copyCFG(cfunc->getCFA(), valmap, func->getCFA(),  locmap, wlist.inserter(), locinfoc,frame);

        for (auto& ne : wlist) {
	  auto& ninstr = ne->getInstructions ();
	  auto ne_from = ne->getFrom ();
	  if (!ninstr)
	    continue;
	  switch (ninstr.last().getOpcode()) {
	  case MiniMC::Model::InstructionCode::Call: 
	    newCall (ne);
	    break;
	    
	  case MiniMC::Model::InstructionCode::RetVoid: {
	    ninstr.last() = Instruction::make<InstructionCode::Skip> (0);
	    cfunc->getCFA ().makeEdge (ne_from,edge->getTo (),std::move(ninstr));
	    cfunc->getCFA().deleteEdge (ne.get());
	    break;
	  }
	    
	  case MiniMC::Model::InstructionCode::Ret: {
	    auto& content = ninstr.last().getOps<MiniMC::Model::InstructionCode::Ret> ();
	    ninstr.last() = Instruction::make<InstructionCode::Assign> ( 
									      call_content.res,
									      content.value 
									 );
				  
	    cfunc->getCFA().makeEdge (ne_from,edge->getTo (),std::move(ninstr));
	    cfunc->getCFA ().deleteEdge (ne.get());
	    break;
	  }
	  default:
	    break;
	  }
        }
	
        
        auto& parameters = cfunc->getParameters();
        
        MiniMC::Model::InstructionStream str;
        for (auto it = instrs.begin(); it != instrs.end() - 1; ++it) {
          str.add(*it);
        }	
	auto it = parameters.begin();
	for (size_t i = 0; i < call_content.params.size (); i++, it++) {
          
          str.add<InstructionCode::Assign> (
					    valmap.at(std::static_pointer_cast<MiniMC::Model::Register> (*it)->getSymbol ()),
					    call_content.params.at(i));  
        }

	cfunc->getCFA ().makeEdge (edge->getFrom(),locmap.at(cfunc->getCFA().getInitialLocation()->getSymbol ()),std::move(str));
	cfunc->getCFA ().deleteEdge (edge.get ());
	
      }

      bool InlineFunctions::runFunction(const MiniMC::Model::Function_ptr& F,std::size_t depth) {
        MiniMC::Model::LocationInfoCreator linfoc(F->getRegisterDescr ());
        MiniMC::Support::WorkingList<std::pair<std::size_t,Edge_ptr>> wlist;
        auto inserter = wlist.inserter();
        auto& cfg = F->getCFA();
	auto unrollFailed = F->getCFA().makeLocation (F->getFrame ().makeFresh (),linfoc.make ({}));
        std::for_each(cfg.getEdges().begin(),
                      cfg.getEdges().end(),
                      [&inserter,depth](const MiniMC::Model::Edge_ptr& e) {
			if (e->getInstructions () && e->getInstructions().last().getOpcode() == MiniMC::Model::InstructionCode::Call)
			  inserter = std::make_pair (depth,e);
		      }
		      );


	/*if (depth - 1) 
	  inlineCallEdgeToFunction(prgm,func, ne, locinfoc, depth - 1,frame);
	else {
	  auto _f =    ne->getFrom ();
	  auto _dloc = func->getCFA ().makeLocation (frame.makeFresh (),locinfoc.make ({}));
	  func->getCFA().makeEdge (_f,_dloc,{},false);
	  func->getCFA().deleteEdge (ne.get());
	  }*/
	
        while (wlist) {
	  auto e = wlist.pop ();
	  auto cdepth = e.first;
	  auto newCallEdge = [&F,cdepth,&inserter,&unrollFailed](auto e) {
	    if (cdepth - 1) 
	      inserter = std::make_pair(cdepth-1,e);//inlineCallEdgeToFunction(prgm,func, ne, locinfoc, depth - 1,frame);
	    else {
	      auto _f =    e->getFrom ();
	      F->getCFA().makeEdge (_f,unrollFailed,{},false);
	      F->getCFA().deleteEdge (e.get());
	    }
	  };
	  
	  
	  inlineCallEdgeToFunction(prgm,F, e.second, linfoc, newCallEdge, F->getFrame ());
	}
	
        return true;
      }


    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC
