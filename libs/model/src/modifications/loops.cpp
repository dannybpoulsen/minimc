#include "model/analysis/loops.hpp"
#include "model/modifications/helpers.hpp"
#include "model/modifications/loops.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include <sstream>

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      template <class LocInserter>
      void unrollLoop(MiniMC::Model::CFA& cfg, const MiniMC::Model::Analysis::Loop* loop, std::size_t amount, LocInserter linserter, MiniMC::Model::LocationInfoCreator& locInf,Frame& frame ) {
        std::vector<SymbolTable<MiniMC::Model::Location_ptr>> unrolledLocations;
	auto inf = locInf.make ( {});
	auto deadLoc = cfg.makeLocation(frame.makeFresh (),inf);
        deadLoc->getInfo().getFlags() |= MiniMC::Model::Attributes::UnrollFailed ;
        for (size_t i = 0; i < amount; i++) {
          std::stringstream str;
          str << "L" << i;
          SymbolTable<MiniMC::Model::Location_ptr> map;
          auto inserter = std::inserter(map, map.begin());
          copyLocation(cfg, loop->getHeader(), inserter, linserter, locInf,frame);

          std::for_each(loop->body_begin(), loop->body_end(), [&](auto& t) { copyLocation(cfg, t, inserter, linserter, locInf,frame); });
          unrolledLocations.push_back(map);
        }

        std::for_each(loop->back_begin(), loop->back_end(), [&](auto& e) {
	  auto instructions = e->getInstructions ();
	  cfg.makeEdge (e->getFrom (),unrolledLocations[0].at(loop->getHeader()->getSymbol ()),std::move(instructions),e->isPhi ());
	  cfg.deleteEdge (e);
	});

        for (size_t i = 0; i < amount; i++) {
          auto& locations = unrolledLocations[i];
          std::for_each(loop->internal_begin(), loop->internal_end(), [&](auto& e) { copyEdge(e, locations, cfg); });
          std::for_each(loop->exiting_begin(), loop->exiting_end(), [&](auto& e) { copyEdge(e, locations, cfg); });

          std::for_each(loop->back_begin(), loop->back_end(), [&](auto& e) {
            auto& locmap = unrolledLocations[i];
            MiniMC::Model::Location_ptr from = locmap.at(e->getFrom()->getSymbol());
            MiniMC::Model::Location_ptr to;

            if (i < amount - 1) {
              to = unrolledLocations[i + 1].at(loop->getHeader()->getSymbol ());
            } else
              to = deadLoc;
            auto nedge = cfg.makeEdge(from, to,MiniMC::Model::InstructionStream{e->getInstructions ()});
            
          });
        }
      }

      bool UnrollLoops::runFunction(const MiniMC::Model::Function_ptr& func,std::size_t maxAmount) {
        MiniMC::Support::Messager{}.message(MiniMC::Support::Localiser("Unrolling Loops for: '%1%'").format(func->getSymbol()));
        auto& cfg = func->getCFA();
	MiniMC::Model::LocationInfoCreator locc {func->getRegisterDescr ()};
        auto loopinfo = MiniMC::Model::Analysis::createLoopInfo(cfg);
        std::vector<MiniMC::Model::Analysis::Loop*> loops;
        loopinfo.enumerate_loops(std::back_inserter(loops));

        for (auto& l : loops) {
          auto parent = l->getParent();
          if (parent) {
            
            unrollLoop(cfg, l, maxAmount, parent->body_insert(), locc,func->getFrame ());
            parent->finalise();
          } else {
            struct Dummy {
              void operator=(MiniMC::Model::Location_ptr) {}
            } dummy;
            unrollLoop(cfg, l, maxAmount, dummy, locc,func->getFrame ());
          }
        }
        return true;
      }

      
    }; // namespace Modifications

  } // namespace Model
} // namespace MiniMC
