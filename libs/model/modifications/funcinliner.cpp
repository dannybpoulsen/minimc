
#include "model/cfg.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/helpers.hpp"
#include "support/sequencer.hpp"
#include "support/pointer.hpp"
#include "support/exceptions.hpp"
#include "support/workinglist.hpp"



namespace MiniMC {
  namespace Model {
    namespace Modifications {

	  void inlineCallEdgeToFunction (const MiniMC::Model::Function_ptr& func, const MiniMC::Model::Edge_ptr& edge,size_t depth = 10) {
		if (!depth)
		  throw MiniMC::Support::Exception ("Inlining Depth exceeded");
		auto from_loc = edge->getFrom ();
		auto to_loc = edge->getTo ();
		auto& instrs = edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		assert(instrs.last().getOpcode () == MiniMC::Model::InstructionCode::Call );
		MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Call>  helper (instrs.last());
		assert (helper.getFunctionPtr()->isConstant ());
		auto constant = std::static_pointer_cast<MiniMC::Model::BinaryBlobConstant > (helper.getFunctionPtr ());
		MiniMC::loadHelper<MiniMC::pointer_t> loadPtr(constant->getData (),sizeof(MiniMC::pointer_t));
				
		auto cfunc = edge->getProgram()->getFunction(MiniMC::Support::getFunctionId (loadPtr));
		MiniMC::Model::Modifications::ReplaceMap<MiniMC::Model::Value> valmap;
		auto copyVar = [&](MiniMC::Model::VariableStackDescr_ptr& stack) { 
		  for (auto& v : stack->getVariables ()) {
			valmap.insert(std::make_pair(v.get(),func->getVariableStackDescr()->addVariable (cfunc->getName()+v->getName(),v->getType ())));
		  }
		  
		  
						   
		};
		
		copyVar (cfunc->getVariableStackDescr ());

		ReplaceMap<MiniMC::Model::Location> locmap;
		std::vector<Location_ptr> nlocs;
		MiniMC::Support::WorkingList<Edge_ptr> wlist;
		
		copyCFG (cfunc->getCFG ().get(), valmap,func->getCFG().get(),cfunc->getName(),locmap,std::back_inserter(nlocs),wlist.inserter());
		
		for (auto& ne :wlist) {
		  if (ne->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			auto& ninstr = ne->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
			if (ninstr.last().getOpcode () == MiniMC::Model::InstructionCode::Call) {
			  inlineCallEdgeToFunction (func,ne,depth-1);
			}

			else if (ninstr.last().getOpcode () == MiniMC::Model::InstructionCode::RetVoid) {
			  InstBuilder<MiniMC::Model::InstructionCode::Skip> builder;
			  ne->setTo (edge->getTo ());
			  ninstr.last().replace (builder.BuildInstruction ());
			}

			else if (ninstr.last().getOpcode () == MiniMC::Model::InstructionCode::Ret) {
			  InstHelper<MiniMC::Model::InstructionCode::Ret> nehelper (ninstr.last());
			  InstBuilder<MiniMC::Model::InstructionCode::Assign> builder;
			  builder.setResult (helper.getResult ());
			  builder.setValue (nehelper.getValue ());
			  ne->setTo (edge->getTo ());
			  ninstr.last().replace(builder.BuildInstruction ());
			}
			
		  }
		}

		edge->setTo (locmap.at(cfunc->getCFG()->getInitialLocation ().get().get()));
		
		auto& parameters = cfunc->getParameters ();
		auto it = parameters.begin();
		MiniMC::Model::InstructionStream str;
		auto inserter = str.back_inserter ();
		for (auto it = instrs.begin(); it != instrs.end()-1; ++it) {
		  inserter = *it;
		}
		for(size_t i = 0; i < helper.nbParams (); i++,it++) {
		  InstBuilder<MiniMC::Model::InstructionCode::Assign> builder;
		  builder.setResult (valmap.at(it->get().get()));
		  builder.setValue (helper.getParam(i));
		  inserter = builder.BuildInstruction ();
		}
		edge->delAttribute<MiniMC::Model::AttributeType::Instructions> ();
		if (str.begin () != str.end())
		  edge->setAttribute<MiniMC::Model::AttributeType::Instructions> (str);
		from_loc->getInfo().template unset<MiniMC::Model::Attributes::CallPlace> ();
	  }
	  
	  bool InlineFunctions::run (MiniMC::Model::Program&  prgm) {
		for (auto& F : prgm.getFunctions ()) {
		  MiniMC::Support::WorkingList<Edge_ptr> wlist;
		  auto inserter = wlist.inserter();
		  auto cfg =  F->getCFG();
		  std::for_each (cfg->getEdges().begin(),
						 cfg->getEdges().end (),
						 [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
						 );
		  for (auto& e : wlist) {
			if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions> () &&
				e->getAttribute<MiniMC::Model::AttributeType::Instructions> ().last().getOpcode () ==
				MiniMC::Model::InstructionCode::Call
				) {
			  inlineCallEdgeToFunction (F,e,depth);
			}
		  }
		  
		}
		return true;
	  }

	}
  }
}
