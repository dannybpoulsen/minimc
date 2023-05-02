#ifndef _HELPERS__
#define _HELPERS__

#include "model/cfg.hpp"
#include "support/exceptions.hpp"
#include "support/sequencer.hpp"
#include <algorithm>
#include <unordered_map>

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      template <class F, class T = std::shared_ptr<F>>
      struct ReplaceMap {
	using value_type = std::unordered_map<F*, T>::value_type;
	using iterator = std::unordered_map<F*, T>::iterator;
	auto at (F* t) const {return map.at (t);}
	auto count (F* t) const {return map.count(t);}

	auto insert (value_type t) {return map.insert(t);}
	auto insert (iterator it, value_type t) {return map.insert(it,t);}
	
	auto begin () {return map.begin ();}
	auto end () {return map.end ();}
	
	std::unordered_map<F*, T> map;
	
      };

      using ValueReplaceMap = ReplaceMap<MiniMC::Model::Value, MiniMC::Model::Value_ptr>;
      using LocationReplaceMap = ReplaceMap<MiniMC::Model::Location, MiniMC::Model::Location_ptr>;
      
      template <class Inserter>
      void copyInstructionAndReplace(const MiniMC::Model::Instruction& inst, const ValueReplaceMap& val, Inserter insert) {
        //std::vector<MiniMC::Model::Value_ptr> vals;
        //auto inserter = std::back_inserter(vals);
	auto replaceFunction = [&](const MiniMC::Model::Value_ptr& op) {
          if (op->isConstant()) {
            return op;
          } else {
            return val.at(op.get());
          }
        }; 
	
        insert = MiniMC::Model::Instruction (inst,replaceFunction);
      }

      inline void copyEdge(const MiniMC::Model::Edge* edge,
                              const LocationReplaceMap& locs,
                              MiniMC::Model::CFA& cfg) {

        auto to = (locs.count(edge->getTo().get())) ? locs.at(edge->getTo().get()) : edge->getTo();
        auto from = (locs.count(edge->getFrom().get())) ? locs.at(edge->getFrom().get()) : edge->getFrom();

        
	auto& orig = edge->getInstructions ();
	MiniMC::Model::InstructionStream nstr (orig);
	cfg.makeEdge(from, to,std::move(nstr),edge->isPhi());
	
	
      }
      
      template <class LocInsert, class LocInserter>
      void copyLocation(MiniMC::Model::CFA& to, const MiniMC::Model::Location_ptr& loc, LocInsert inserter, LocInserter linserter, MiniMC::Model::LocationInfoCreator& linfo, Frame frame) {
	auto info = linfo.make(loc->getInfo());
	auto nloc = to.makeLocation(frame.makeFresh (),info);
        inserter = std::make_pair(loc.get(), nloc);
        linserter = nloc;
      }
      
      template <class Inserter>
      void copyEdgeAndReplace(const MiniMC::Model::Edge_ptr& edge,
                              const ValueReplaceMap& val,
                              const LocationReplaceMap& locs,
                              MiniMC::Model::CFA& cfg,
                              Inserter insertTo) {
        auto to = (locs.count(edge->getTo().get())) ? locs.at(edge->getTo().get()) : edge->getTo();
        auto from = (locs.count(edge->getFrom().get())) ? locs.at(edge->getFrom().get()) : edge->getFrom();


	auto& orig = edge->getInstructions ();
	MiniMC::Model::InstructionStream nstr;
	std::for_each(orig.begin(), orig.end(), [&](const MiniMC::Model::Instruction& inst) {
	  auto replaceF = [&](const MiniMC::Model::Value_ptr& op) {
	    return val.count(op.get()) ? val.at (op.get ()) : op;
	  };
	  
	  nstr.add(Instruction (inst,replaceF));
	});

	
        insertTo = cfg.makeEdge(from, to,std::move(nstr),edge->isPhi ());
      }

      template <class LocInsert, class EdgeInsert>
      void copyCFG(const MiniMC::Model::CFA& from,
                   ValueReplaceMap& val,
                   MiniMC::Model::CFA& to,
                   LocationReplaceMap& locmap,
                   LocInsert lInsert,
                   EdgeInsert eInsert,
                   MiniMC::Model::LocationInfoCreator& locinfoc,
		   MiniMC::Model::Frame frame
		   ) {
        for (auto& loc : from.getLocations()) {
	  auto info = locinfoc.make(loc->getInfo());
          auto nloc = to.makeLocation(frame.makeSymbol (loc->getSymbol ().getName ()),info);
          locmap.insert(std::pair(loc.get(), nloc));
          lInsert = loc;
        }

        for (auto& e : from.getEdges()) {
          copyEdgeAndReplace<EdgeInsert>(e, val, locmap, to, eInsert);
        }
      }

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
