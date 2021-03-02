#include <sstream>
#include "model/analysis/loops.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/helpers.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {

	  template<class LocInserter>
	  void unrollLoop (MiniMC::Model::CFG_ptr cfg, const MiniMC::Model::Analysis::Loop* loop,std::size_t amount, LocInserter linserter, MiniMC::Model::Program& prgm) {
		std::vector<ReplaceMap<MiniMC::Model::Location>> unrolledLocations;
		auto deadLoc = cfg->makeLocation ({"DEAD"}).get();
		//linserter =deadLoc;
		for (size_t i = 0; i< amount; i++) {
		  std::stringstream str;
		  str << "L" << i;
		  ReplaceMap<MiniMC::Model::Location> map;
		  auto inserter = std::inserter (map,map.begin());
		  copyLocation (cfg,loop->getHeader (),inserter,linserter,str.str());
		  
		  std::for_each (loop->body_begin(), loop->body_end (),[&](auto& t) {copyLocation (cfg,t,inserter,linserter,str.str());});
		  unrolledLocations.push_back (map);
		  
		}

		std::for_each (loop->back_begin(),loop->back_end(),[&] (auto& e) {
			e->setTo (unrolledLocations[0][loop->getHeader().get()]);
		  });
		
		for (size_t i = 0; i < amount; i++) {
		  auto& locations =  unrolledLocations[i];
		  std::for_each (loop->internal_begin(),loop->internal_end(),[&](auto& e) {copyEdgeAnd (e,locations,cfg);});
		  std::for_each (loop->exiting_begin(),loop->exiting_end(),[&](auto& e) {copyEdgeAnd (e,locations,cfg);});
		  
		  
		  std::for_each (loop->back_begin(),loop->back_end(),[&] (auto& e) {
			  auto&locmap = unrolledLocations[i];
			  MiniMC::Model::Location_ptr from = locmap.at (e->getFrom().get ().get());  
			  MiniMC::Model::Location_ptr to;

			  if (i < amount-1) {
				to = unrolledLocations[i+1].at (loop->getHeader().get());
			  }
			  else
				to = deadLoc;
			  auto nedge = cfg->makeEdge (from,to);
			  nedge->copyAttributesFrom (*e);
			});
		  
		}
		
		  
		
	  }
	  
	  
	  bool UnrollLoops::run (MiniMC::Model::Program&  prgm) {
		for (auto& func : prgm.getEntryPoints ()) {
		  auto cfg = func->getCFG ();
		  
		  auto loopinfo = MiniMC::Model::Analysis::createLoopInfo (cfg);
		  std::vector<MiniMC::Model::Analysis::Loop*> loops;
		  loopinfo.enumerate_loops (std::back_inserter (loops));
		  
		  for (auto& l : loops) { 
			auto parent = l->getParent ();
			if (parent)  {
			  struct Dummy {
				void operator= (MiniMC::Model::Location_ptr) {}
			  } dummy;
			  
			  unrollLoop (cfg,l,maxAmount,parent->body_insert(),prgm);
			  parent->finalise();
			}
			else {
			  struct Dummy {
				void operator= (MiniMC::Model::Location_ptr) {}
			  } dummy;
			  unrollLoop (cfg,l,maxAmount,dummy,prgm);
			}
			  
		  }
		  
		}
		 
		return true;
	  }
	};
	
  }
}

