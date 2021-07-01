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
			  if (E->getFrom ()->getInfo().template is<MiniMC::Model::Attributes::CallPlace> () &&
				  E->template getAttribute<MiniMC::Model::AttributeType::Instructions> ().last ().getOpcode () !=
				  MiniMC::Model::InstructionCode::Call) 
				{
				  E->getFrom()->getInfo().unset<MiniMC::Model::Attributes::CallPlace> ();
				}
			}
		  }
		  return true;
		}
      };

	  struct ConvergencePointAnnotator : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  auto source_loc = std::make_shared<MiniMC::Model::SourceInfo> ();
		  for (auto& F : prgm.getFunctions ()) {
			MiniMC::Model::LocationInfoCreator locinfoc(F->getName ());
			MiniMC::Support::WorkingList<MiniMC::Model::Location_ptr> wlist;
			auto inserter =wlist.inserter ();
			auto cfg = F->getCFG ();
			std::for_each (cfg->getLocations().begin(),
						   cfg->getLocations().end (),
						   [&](const MiniMC::Model::Location_ptr& l) {inserter = l;}
						   );
			while (!wlist.empty ()) {
			  MiniMC::Model::Location_ptr location = wlist.pop ();
			  if (location->nbIncomingEdges () == 2) {
				location->getInfo().set<MiniMC::Model::Attributes::ConvergencePoint> ();
			  }
			  else if (location->nbIncomingEdges () > 2) {
				auto nlocation = cfg->makeLocation (locinfoc.make("",0,*source_loc));
				nlocation->getInfo().set<MiniMC::Model::Attributes::ConvergencePoint> ();
				auto it = location->ebegin ();
				MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> elist;
				std::for_each (location->ebegin(),
							   location->eend (),
							   [&](const MiniMC::Model::Edge_ptr& l) {elist.inserter() = l;}
							   );
				for (auto& edge : elist) {
				  auto nedge = cfg->makeEdge (nlocation,edge->getTo ());
				  nedge->copyAttributesFrom (**it);
				  cfg->deleteEdge (edge);
				}
				cfg->makeEdge (location,nlocation);
				auto eit = location->iebegin ();
				eit->setTo (nlocation);
				wlist.inserter() = location;
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
		  auto addToWorkingList = [](MiniMC::Support::WorkingList<MiniMC::Model::Edge_wptr>& wlist, auto beg, auto end) {
			auto inserter = wlist.inserter ();
			std::for_each (beg,
						   end,
						   [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
						   );
		  };

		  auto canSkipLocation =  [](const MiniMC::Model::Location_ptr& loc) {
			return loc->nbIncomingEdges () <= 1 &&
			  !loc->getInfo().template is<MiniMC::Model::Attributes::AssumptionPlace> () &&
			  !loc->getInfo().template  is<MiniMC::Model::Attributes::CallPlace> ();
										
		  };
		  
		  for (auto& F : prgm.getFunctions ()) {
			bool modified = false;
			do {
		  
			  modified = false;
			  auto cfg = F->getCFG ();

			  
			  MiniMC::Support::WorkingList<MiniMC::Model::Edge_wptr> wlist;
			  addToWorkingList (wlist,cfg->getEdges ().begin(),cfg->getEdges().end ());
			  bool inner_mod = false;
			  for (auto& EW : wlist) {
				inner_mod = false;
				auto edge = EW.lock ();
				if (edge) {
				  auto from = edge->getFrom ();
				  auto to = edge->getTo ();
				  if (!from->getInfo().template is<MiniMC::Model::Attributes::CallPlace> () &&
					  canSkipLocation (to)) {
					MiniMC::Support::WorkingList<MiniMC::Model::Edge_wptr> inner_wlist;
					addToWorkingList (inner_wlist,to->ebegin (),to->eend ());
		    
					for (auto rwedge : inner_wlist) {
					  if (auto remove_edge = rwedge.lock () ) {
						inner_mod = true;
						auto nedge = cfg->makeEdge (from,remove_edge->getTo ());
						
						copyInstrStream (nedge,edge);
						copyInstrStream (nedge,remove_edge);
						
						cfg->deleteEdge (remove_edge);
		
					  }
					}
				  }
				  modified |=inner_mod;
		  
				}
				if (inner_mod) {
				  cfg->deleteEdge (edge);
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
		  auto source_loc = std::make_shared<MiniMC::Model::SourceInfo> ();
		  for (auto& F : prgm.getFunctions ()) {
			MiniMC::Model::LocationInfoCreator locinfoc(F->getName ());
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
				  auto nloc = cfg->makeLocation (locinfoc.make("",0,*source_loc));
				  auto nedge = cfg->makeEdge (from,nloc);
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

      using EnsureEdgesOnlyHasOneCompar = EnsureEdgesOnlyHasOne<MiniMC::Model::InstructionCode::ICMP_SGT,
																MiniMC::Model::InstructionCode::ICMP_SGE,
																MiniMC::Model::InstructionCode::ICMP_SLT,
																MiniMC::Model::InstructionCode::ICMP_SLE,
																MiniMC::Model::InstructionCode::ICMP_EQ,
																MiniMC::Model::InstructionCode::ICMP_NEQ,
																MiniMC::Model::InstructionCode::ICMP_UGT,
																MiniMC::Model::InstructionCode::ICMP_UGE,
																MiniMC::Model::InstructionCode::ICMP_ULT,
																MiniMC::Model::InstructionCode::ICMP_ULE>;
      
    }
  }
}


#endif 
