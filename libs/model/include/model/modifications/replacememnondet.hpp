/**
 * @file   replacememnondet.hpp
 * @date   Mon Apr 20 17:05:18 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _REPLACEMEM__
#define _REPLACEMEM__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "model/modifications/simplify_cfg.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
	  /**
	   * Translates all memory  \ref MiniMC::Model::InstructionCode::Load instructions to  \ref MiniMC::Model::InstructionCode::NonDet instrucitons.
	   * This is useful when an analysis engine does not model, but only works on registers. 
	   **/
      struct RemoveMemNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
		    for (auto& E : F->getCFG()->getEdges ()) {
		      if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			  if (I.getOpcode () == MiniMC::Model::InstructionCode::Load ) {
			    MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::NonDet> nondet;
			    MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Load> load (I);	  
			    nondet.setResult (load.getResult ());
			    I.replace (nondet.BuildInstruction ());
			  }
			  if (I.getOpcode () == MiniMC::Model::InstructionCode::Alloca ||
				  I.getOpcode () == MiniMC::Model::InstructionCode::Malloc ||
				  I.getOpcode () == MiniMC::Model::InstructionCode::ExtendObj || 
				  I.getOpcode () == MiniMC::Model::InstructionCode::Store  
				  
				  ) {
				MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Skip> skip;
				I.replace (skip.BuildInstruction ());
			  }
			  
			}
		      }
		    }
		  }
		  return true;
		}
	
      };
	  
	  
      struct ExpandNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  //First make sure we only have one NonDet on each edge
		  EnsureEdgesOnlyHasOne<MiniMC::Model::InstructionCode::NonDet>{}.run (prgm);
		  
		  for (auto& F : prgm.getFunctions ()) {
			std::vector<MiniMC::Model::Edge> todel;
			auto cfg =  F->getCFG();
			MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
			auto inserter =wlist.inserter ();
			std::for_each (cfg->getEdges().begin(),
						   cfg->getEdges().end (),
						   [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
						   );
			
			for (auto& E : wlist) {
			  if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				auto& origstr = E->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
				auto& instr = origstr.last();	  
				
				if (instr.getOpcode () == MiniMC::Model::InstructionCode::NonDet) {
				  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::NonDet> nondet (instr);	  
				  assert(nondet.getResult ()->getType ()->getTypeID () == MiniMC::Model::TypeID::Integer);
				  auto type = nondet.getResult ()->getType ();
				  auto from = E->getFrom();
				  auto to = E->getTo ();
				  MiniMC::uint64_t min = 0;
				  MiniMC::uint64_t max = 0;
				  
				  
				  switch (type->getSize()) {
				  case 1:
					min =std::numeric_limits<MiniMC::uint8_t>::min ();
					max =std::numeric_limits<MiniMC::uint8_t>::max ();
					break;
				  case 2:
					min =std::numeric_limits<MiniMC::uint16_t>::min ();
					max =std::numeric_limits<MiniMC::uint16_t>::max ();
					break;
				  case 4:
					min =std::numeric_limits<MiniMC::uint32_t>::min ();
					max =std::numeric_limits<MiniMC::uint32_t>::max ();
					break;
				  case 8:
					min =std::numeric_limits<MiniMC::uint64_t>::min ();
					max =std::numeric_limits<MiniMC::uint64_t>::max ();
					break;
				  default:
					assert(false);
				  }
				  auto& fact = prgm.getConstantFactory ();
				  MiniMC::uint64_t it = min;
				  
				  while (true) {
					MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assign> builder;
					auto val = fact->makeIntegerConstant (it,type);
					builder.setResult (nondet.getResult ());
					builder.setValue (val);

					auto nedge = cfg->makeEdge (from,to);
					nedge->setAttribute<MiniMC::Model::AttributeType::Instructions> (origstr);
					nedge->getAttribute<MiniMC::Model::AttributeType::Instructions>().last().replace (builder.BuildInstruction ());
					it++;
					if (it == max)
					  break;
				  }
				  cfg->deleteEdge (E);
				}
				

				
				
			  }
			}
			
		  }

		  return true;
		}
	  };
		
	}
  }


}

#endif
