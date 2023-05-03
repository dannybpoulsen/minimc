
#include "model/cfg.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/helpers.hpp"
#include "support/exceptions.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      void inlineCallEdgeToFunction(const MiniMC::Model::Program& prgm, const MiniMC::Model::Function_ptr& func, const MiniMC::Model::Edge_ptr& edge, MiniMC::Model::LocationInfoCreator& locinfoc, size_t depth, MiniMC::Model::Frame cframe) {
        if (!depth)
          //throw MiniMC::Support::Exception("Inlining Depth exceeded");
	
        auto from_loc = edge->getFrom();
        auto to_loc = edge->getTo();
        auto& instrs = edge->getInstructions();
        assert(instrs.last().getOpcode() == MiniMC::Model::InstructionCode::Call);
	auto call_content = instrs.last ().getOps<MiniMC::Model::InstructionCode::Call> ();
	auto constant = std::static_pointer_cast<MiniMC::Model::Pointer>(call_content.function);
        MiniMC::pointer_t loadPtr =  constant->getValue (); 
	auto cfunc = prgm.getFunction(MiniMC::getFunctionId(loadPtr));
	auto frame = cframe.create (cfunc->getSymbol ().getName ());
	MiniMC::Model::Modifications::ValueReplaceMap valmap;
        auto copyVar = [&](MiniMC::Model::RegisterDescr& stack) {
          for (auto& v : stack.getRegisters()) {
            valmap.insert(std::make_pair(v->getSymbol (), func->getRegisterDescr().addRegister(frame.makeSymbol (v->getSymbol ().getName ()), v->getType())));
          }
        };

        copyVar(cfunc->getRegisterDescr());

        LocationReplaceMap locmap;
        std::vector<Location_ptr> nlocs;
        MiniMC::Support::WorkingList<Edge_ptr> wlist;

        copyCFG(cfunc->getCFA(), valmap, func->getCFA(),  locmap, std::back_inserter(nlocs), wlist.inserter(), locinfoc,frame);

        for (auto& ne : wlist) {
	  auto& ninstr = ne->getInstructions ();
	  auto ne_from = ne->getFrom ();
	  if (!ninstr)
	    continue;
	  if (ninstr.last().getOpcode() == MiniMC::Model::InstructionCode::Call) {
	    inlineCallEdgeToFunction(prgm,func, ne, locinfoc, depth - 1,frame);
	  }
	  
	  else if (ninstr.last().getOpcode() == MiniMC::Model::InstructionCode::RetVoid) {
	    ninstr.last().replace (Instruction::make<InstructionCode::Skip> (0));
	    cfunc->getCFA ().makeEdge (ne_from,edge->getTo (),std::move(ninstr));
	    cfunc->getCFA().deleteEdge (ne.get());
	  }
	  
	  else if (ninstr.last().getOpcode() == MiniMC::Model::InstructionCode::Ret) {
	    auto& content = ninstr.last().getOps<MiniMC::Model::InstructionCode::Ret> ();
	    ninstr.last().replace(Instruction::make<InstructionCode::Assign> ( 
									      call_content.res,
									      content.value 
									       )
				  );
	    cfunc->getCFA().makeEdge (ne_from,edge->getTo (),std::move(ninstr));
	    cfunc->getCFA ().deleteEdge (ne.get());
	    
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
        MiniMC::Support::WorkingList<Edge_ptr> wlist;
        auto inserter = wlist.inserter();
        auto& cfg = F->getCFA();
        std::for_each(cfg.getEdges().begin(),
                      cfg.getEdges().end(),
                      [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
        for (auto& e : wlist) {
          if (e->getInstructions () &&
              e->getInstructions().last().getOpcode() ==
	      MiniMC::Model::InstructionCode::Call) {
            inlineCallEdgeToFunction(prgm,F, e, linfoc, depth, F->getFrame ());
          }
        }

        return true;
      }


    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC
