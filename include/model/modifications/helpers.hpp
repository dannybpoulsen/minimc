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
      template <class T>
      using ReplaceMap = std::unordered_map<T*, std::shared_ptr<T>>;

      template <class Inserter>
      void copyInstructionAndReplace(const MiniMC::Model::Instruction& inst, const ReplaceMap<MiniMC::Model::Value>& val, Inserter insert) {
        //std::vector<MiniMC::Model::Value_ptr> vals;
        //auto inserter = std::back_inserter(vals);
	auto replaceFunction = [&](const MiniMC::Model::Value_ptr& op) {
          if (op->isConstant()) {
            return op;
          } else {
            return val.at(op.get());
          }
        }; 
	
        insert = MiniMC::Model::copyInstructionWithReplace (inst,replaceFunction);
      }

      inline void copyEdgeAnd(const MiniMC::Model::Edge* edge,
                              const ReplaceMap<MiniMC::Model::Location>& locs,
                              MiniMC::Model::CFA& cfg) {

        auto to = (locs.count(edge->getTo().get())) ? locs.at(edge->getTo().get()) : edge->getTo();
        auto from = (locs.count(edge->getFrom().get())) ? locs.at(edge->getFrom().get()) : edge->getFrom();

        auto nedge = cfg.makeEdge(from, to);
        
        if (edge->getInstructions ()) {
          auto& orig = edge->getInstructions ();
          MiniMC::Model::InstructionStream nstr (orig.isPhi ());
          std::for_each(orig.begin(), orig.end(), [&](const MiniMC::Model::Instruction& inst) {
            nstr.addInstruction (inst); 
          });

          nedge->getInstructions () = nstr;
        }
      }

      template <class LocInsert, class LocInserter>
      void copyLocation(MiniMC::Model::CFA& to, const MiniMC::Model::Location_ptr& loc, LocInsert inserter, LocInserter linserter, MiniMC::Model::LocationInfoCreator& linfo) {
        auto nloc = to.makeLocation(linfo.make(loc->getInfo()));
        inserter = std::make_pair(loc.get(), nloc);
        linserter = nloc;
      }
      template <class T>
      auto lookupValue(std::shared_ptr<T> v, const ReplaceMap<T>& map) {
        if (map.count(v.get())) {
          return map.at(v.get());
        } else {
          return v;
        }
      }

      template <class Inserter>
      void copyEdgeAndReplace(const MiniMC::Model::Edge_ptr& edge,
                              const ReplaceMap<MiniMC::Model::Value>& val,
                              const ReplaceMap<MiniMC::Model::Location>& locs,
                              MiniMC::Model::CFA& cfg,
                              Inserter insertTo) {
        auto to = (locs.count(edge->getTo().get())) ? locs.at(edge->getTo().get()) : edge->getTo();
        auto from = (locs.count(edge->getFrom().get())) ? locs.at(edge->getFrom().get()) : edge->getFrom();

        auto nedge = cfg.makeEdge(from, to);
        
        if (edge->getInstructions ()) {
          auto& orig = edge->getInstructions ();
          MiniMC::Model::InstructionStream nstr (orig.isPhi ());
          std::for_each(orig.begin(), orig.end(), [&](const MiniMC::Model::Instruction& inst) {
	    auto replaceF = [&](const MiniMC::Model::Value_ptr& op) {
	      return lookupValue (op,val);
	    };
	      
	    nstr.addInstruction(copyInstructionWithReplace (inst,replaceF));
          });

          nedge->getInstructions () = std::move(nstr);
        }

        insertTo = nedge;
      }

      template <class LocInsert, class EdgeInsert>
      void copyCFG(const MiniMC::Model::CFA& from,
                   ReplaceMap<MiniMC::Model::Value>& val,
                   MiniMC::Model::CFA& to,
                   ReplaceMap<MiniMC::Model::Location>& locmap,
                   LocInsert lInsert,
                   EdgeInsert eInsert,
                   MiniMC::Model::LocationInfoCreator& locinfoc) {
        for (auto& loc : from.getLocations()) {
          auto nloc = to.makeLocation(locinfoc.make(loc->getInfo()));
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
