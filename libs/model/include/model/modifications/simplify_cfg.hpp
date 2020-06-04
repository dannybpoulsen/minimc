/**
 * @file   simplify_cfg.hpp
 * @date   Mon Apr 20 17:09:28 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _SIMPLIFYCFG__
#define _SIMPLIFYCFG__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      inline void  copyInstrStream (const MiniMC::Model::Edge_ptr& to, const MiniMC::Model::Edge_ptr& copyee) {
		if (copyee->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  if (!to->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			MiniMC::Model::InstructionStream str;
			to-> template setAttribute<MiniMC::Model::AttributeType::Instructions> (str);
		  }
		  auto& str = to->template getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		  auto backInsert = str.back_inserter ();
		  for (auto& e : copyee->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			backInsert = e;
		  }
		}
      }

	  /**
	   * Remove \ref MiniMC::Model::Location::Attributes::CallPlace annotations which are not really call-places
	   */
      struct RemoveUnneededCallPlaceAnnotations : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
			auto inserter =wlist.inserter ();
			auto cfg = F->getCFG ();
			std::for_each (cfg->getEdges().begin(),
						   cfg->getEdges().end (),
						   [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
						   );
			for (auto& E : wlist) {
			  if (E->getFrom ()->template is<MiniMC::Model::Location::Attributes::CallPlace> () &&
				  E->template getAttribute<MiniMC::Model::AttributeType::Instructions> ().last ().getOpcode () !=
				  MiniMC::Model::InstructionCode::Call) 
				{
				  E->getFrom()->unset<MiniMC::Model::Location::Attributes::CallPlace> ();
				}
			}
		  }
		  return true;
		}
      };
	
      /**
	   * Simplify the CFG by merging edges that can be executed in succession. 
	   * Careful if used with parallel programs, as it does break the interleaving semantics.
	   *
	   */
      struct SimplifyCFG : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			bool modified = false;
			do {
			  modified = false;
			  MiniMC::Support::WorkingList<MiniMC::Model::Edge_wptr> wlist;
			  auto inserter =wlist.inserter ();
			  auto cfg = F->getCFG ();
			  std::for_each (cfg->getEdges().begin(),
							 cfg->getEdges().end (),
							 [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
							 );
			  for (auto& EW : wlist) {
				if (auto E = EW.lock ()) {
				  auto from = E->getFrom ();
				  auto to = E->getTo ();		  
				  if (!from->template is<MiniMC::Model::Location::Attributes::CallPlace> ()) {
					if (to->nbIncomingEdges () <= 1 &&
						!to->template is<MiniMC::Model::Location::Attributes::AssumptionPlace> () 

						)
					  {
			
						MiniMC::Support::WorkingList<MiniMC::Model::Edge_wptr> inner_wlist;
						auto inserter =inner_wlist.inserter ();
						std::for_each (to->ebegin(),
									   to->eend (),
									   [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
									   );
						bool inner_mod = false;
			
						for (auto cwedge : inner_wlist) {
						  if (auto cedge = cwedge.lock ()) {
							if (to->template is<MiniMC::Model::Location::Attributes::CallPlace> ())
							  from->template set<MiniMC::Model::Location::Attributes::CallPlace> ();
							inner_mod = true;
							auto nedge = cfg->makeEdge (from,cedge->getTo (),prgm.shared_from_this());
							copyInstrStream (nedge,E);
							copyInstrStream (nedge,cedge);
							cfg->deleteEdge (cedge);
						  }
						}
						modified |= inner_mod;;
						if (inner_mod)
						  cfg->deleteEdge (E);
					  }
				  }
				}
		
			  }
			}while (modified);
	    
		  }
		  return true;
		}
      };

	  
	  template<MiniMC::Model::InstructionCode... pos>
	  struct EnsureEdgesOnlyHasOne : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			MiniMC::Support::WorkingList<MiniMC::Model::Edge_wptr> wlist;
			auto inserter =wlist.inserter ();
			auto cfg = F->getCFG ();
			std::for_each (cfg->getEdges().begin(),
						   cfg->getEdges().end (),
						   [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
						   );
			for (auto& cwedge :  wlist) {
			  auto edge = cwedge.lock ();
			  assert(edge);
			  std::vector<MiniMC::Model::Edge_ptr> newedges;
			  if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {		
				MiniMC::Model::InstructionStream str;
				auto backInsert = str.back_inserter ();
				
				auto from = edge->getFrom ();
				auto makeEdge = [&]  (MiniMC::Model::InstructionStream& str) {
								  auto nloc = cfg->makeLocation ("");
								  auto nedge = cfg->makeEdge (from,nloc,edge->getProgram());
								  nedge->template setAttribute<MiniMC::Model::AttributeType::Instructions> (str);
								  newedges.push_back (nedge);
								  from = nloc;
								};
				
				for (auto instr : edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				  backInsert = instr;
				  if (MiniMC::Model::isOneOf<pos...> (instr))  {				 
					makeEdge (str);
					str.instr.clear();
					backInsert = str.back_inserter ();
				  }
				  
				}
				
				if (str.instr.size()) {
				  makeEdge (str);
				}
				newedges.back ()->setTo (edge->getTo ());
				cfg->deleteEdge (edge);
			  }
		  }
			
			
		}
		  return true;
		}
      };

	  using EnsureEdgesOnlyHasOneMemAccess = EnsureEdgesOnlyHasOne<MiniMC::Model::InstructionCode::Store,
																   MiniMC::Model::InstructionCode::Load>;
	  
    }
  }
}


#endif 
