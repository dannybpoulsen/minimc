
#include "minimc/model/cfg.hpp"
#include "minimc/model/modifications/func_inliner.hpp"
#include "helpers.hpp"
#include "minimc/model/source.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/support/workinglist.hpp"
#include "minimc/support/overload.hpp"

#include <functional>

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      void inlineCallEdgeToFunction(const MiniMC::Model::Program& prgm, const MiniMC::Model::Function_ptr& func, const MiniMC::Model::Edge_ptr& edge,   std::function<void(MiniMC::Model::Edge_ptr)> newCall, MiniMC::Model::Frame cframe) {
        
        auto from_loc = edge->getFrom();
        auto to_loc = edge->getTo();
        auto instrs = edge->getInstructions();
        auto call_content = instrs.last ().getAs<MiniMC::Model::VMInstructionCode::Call>().getOps ();

	auto cfunc = MiniMC::Model::visitValue(
									   MiniMC::Support::Overload{
									     [&prgm](const MiniMC::Model::Pointer& t) -> MiniMC::Model::Function_ptr {
									       auto loadPtr = t.getValue();
									       return prgm.getFunction(loadPtr.base);
									     },
									       [&prgm](const MiniMC::Model::Pointer32& t) -> MiniMC::Model::Function_ptr {
										 auto loadPtr = t.getValue();
										 return prgm.getFunction(loadPtr.base);
									       },
									       [&prgm](const MiniMC::Model::SymbolicConstant& t) -> MiniMC::Model::Function_ptr {
										 auto symb = t.getValue();
										 return prgm.getFunction(symb);
									       },
									       MiniMC::Support::Error<MiniMC::Model::Function_ptr> {}
									   },
									   *call_content.function
									    );
	
	
	
	MiniMC::Model::SymbolTable<MiniMC::Model::Value_ptr> valmap;
        
	for (auto& v : cfunc->getRegisterDescr().getRegisters()) {
	  valmap.insert(std::make_pair(v.getSymbol (), func->getRegisterDescr().addRegister(cframe.makeFresh (v.getSymbol ().getName ()), v.getType())));
	}

	
	MiniMC::Model::SymbolTable<MiniMC::Model::Location_ptr> locmap;
        MiniMC::Support::WorkingList<Edge_ptr> wlist;
	
        copyCFG(cfunc->getCFA(), valmap, func->getCFA(),  locmap, wlist.inserter(), cframe);

        for (auto& ne : wlist) {
	  auto ninstr = ne->getInstructions ();
	  auto ne_from = ne->getFrom ();
	  if (!ninstr) {
	    continue;
	  }
	  ninstr.last ().visit (MiniMC::Support::Overload {
	      [&ne,newCall](const MiniMC::Model::TInstruction<MiniMC::Model::VMInstructionCode::Call>&) {
		newCall (ne);
	      },
	      [&ne,newCall,&edge,&func,&ne_from,&ninstr](const MiniMC::Model::TInstruction<MiniMC::Model::VMInstructionCode::RetVoid>&) {
		ninstr.last() = Instruction::make<VMInstructionCode::Skip> ();
		func->getCFA ().makeEdge (ne_from,edge->getTo (),std::move(ninstr));
		func->getCFA().deleteEdge (ne.get());		
	},
	      [&func,&ninstr,&edge,&call_content,&ne_from,&ne](const MiniMC::Model::TInstruction<MiniMC::Model::VMInstructionCode::Ret>& instr) {
		auto& content = instr.getOps ();
		ninstr.last() = Instruction::make<VMInstructionCode::Assign> ( 
									      call_content.res,
									    content.value 
									       );	
		func->getCFA().makeEdge (ne_from,edge->getTo (),std::move(ninstr));
		func->getCFA ().deleteEdge (ne.get());
		
		
	      },
		[](auto& ) {}
		}
	    );
	}
	auto& parameters = cfunc->getParameters();
        
        MiniMC::Model::InstructionStream str;
        for (auto it = instrs.begin(); it != instrs.end() - 1; ++it) {
          str.add(*it);
        }	
	auto it = parameters.begin();
	for (size_t i = 0; i < call_content.params.size (); i++, it++) {
          
          str.add<VMInstructionCode::Assign> (
					    valmap.at(std::static_pointer_cast<MiniMC::Model::Register> (*it)->getSymbol ()),
					    call_content.params.at(i));  
        }

	func->getCFA ().makeEdge (edge->getFrom(),locmap.at(cfunc->getCFA().getInitialLocation()->getSymbol ()),std::move(str));
	func->getCFA ().deleteEdge (edge.get ());
	
      }
      

      bool InlineFunctions::runFunction(const MiniMC::Model::Function_ptr& F,std::size_t depth) {
        MiniMC::Support::WorkingList<std::pair<std::size_t,Edge_ptr>> wlist;
        auto inserter = wlist.inserter();
        auto& cfg = F->getCFA();
	auto unrollFailed = F->getCFA().makeLocation (F->getFrame ().makeFresh ("UnrollFailed"),MiniMC::Model::LocationInfo{{},F->getFrame()});
        std::for_each(cfg.getEdges().begin(),
                      cfg.getEdges().end(),
                      [&inserter,depth](const MiniMC::Model::Edge_ptr& e) {
			if (e->getInstructions ()){
			  e->getInstructions().last().visit (MiniMC::Support::Overload {

			      [&inserter,depth,&e](const MiniMC::Model::TInstruction<MiniMC::Model::VMInstructionCode::Call>& ) {
				inserter = std::make_pair (depth,e);
			      },
			      [](auto&) {}
				});
			}
		      }
		      );
	
	
        while (wlist) {
	  auto e = wlist.pop ();
	  auto cdepth = e.first;
	  auto newCallEdge = [&F,cdepth,&inserter,&unrollFailed](auto e) {
	    if (cdepth - 1) 
	      inserter = std::make_pair(cdepth-1,e);
	    else {
	      auto _f = e->getFrom ();
	      F->getCFA().makeEdge (_f,unrollFailed,{},false);
	      F->getCFA().deleteEdge (e.get());
	    }
	  };
	  
	  
	  inlineCallEdgeToFunction(*prgm,F, e.second, newCallEdge, F->getFrame ());
	}
	
        return true;
      }


    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC
