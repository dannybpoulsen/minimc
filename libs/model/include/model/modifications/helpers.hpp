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
      using ValueReplaceMap = MiniMC::Model::SymbolTable<MiniMC::Model::Value_ptr>;//ReplaceMap<MiniMC::Model::Value, MiniMC::Model::Value_ptr>;
      using LocationReplaceMap = MiniMC::Model::SymbolTable<MiniMC::Model::Location_ptr>;//ReplaceMap<MiniMC::Model::Location, MiniMC::Model::Location_ptr>;


      inline void copyEdge(const MiniMC::Model::Edge* edge,
                              const LocationReplaceMap& locs,
                              MiniMC::Model::CFA& cfg) {

        auto to = (locs.count(edge->getTo()->getSymbol())) ? locs.at(edge->getTo()->getSymbol()) : edge->getTo();
        auto from = (locs.count(edge->getFrom()->getSymbol())) ? locs.at(edge->getFrom()->getSymbol()) : edge->getFrom();

        
	auto& orig = edge->getInstructions ();
	MiniMC::Model::InstructionStream nstr (orig);
	cfg.makeEdge(from, to,std::move(nstr),edge->isPhi());
	
	
      }
      
      template <class LocInsert, class LocInserter>
      void copyLocation(MiniMC::Model::CFA& to, const MiniMC::Model::Location_ptr& loc, LocInsert inserter, LocInserter linserter, MiniMC::Model::LocationInfoCreator& linfo, Frame frame) {
	auto info = linfo.make(loc->getInfo());
	auto nloc = to.makeLocation(frame.makeFresh (),info);
        inserter = std::make_pair(loc->getSymbol (), nloc);
        linserter = nloc;
      }
      
      template <class Inserter>
      void copyEdgeAndReplace(const MiniMC::Model::Edge_ptr& edge,
                              const ValueReplaceMap& val,
                              const LocationReplaceMap& locs,
                              MiniMC::Model::CFA& cfg,
                              Inserter insertTo) {
        auto to = (locs.count(edge->getTo()->getSymbol())) ? locs.at(edge->getTo()->getSymbol ()) : edge->getTo();
        auto from = (locs.count(edge->getFrom()->getSymbol())) ? locs.at(edge->getFrom()->getSymbol()) : edge->getFrom();


	auto& orig = edge->getInstructions ();
	MiniMC::Model::InstructionStream nstr;
	std::for_each(orig.begin(), orig.end(), [&](const MiniMC::Model::Instruction& inst) {
	  auto replaceF = [&](const MiniMC::Model::Value_ptr& op)-> MiniMC::Model::Value_ptr {
	    if (!op)
	      return nullptr;
	    if (op->isConstant ())
	      return op;
	    auto reg = std::static_pointer_cast<MiniMC::Model::Register> (op);
	    return val.count(reg->getSymbol ()) ? val.at (reg->getSymbol ()) : op;
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
          locmap.insert(std::pair(loc->getSymbol (), nloc));
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
