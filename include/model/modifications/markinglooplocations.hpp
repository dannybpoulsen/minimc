/**
 * @file   markinglooplocations.hpp
 * @date   Mon Apr 20 17:05:14 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _MARKLOOPLOCS__
#define _MARKLOOPLOCS__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      /** 
	   * Marks only one state from each loop to be important for storage.  
	   * 
	   */
      struct MarkLoopStates : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool run(MiniMC::Model::Program& prgm);
      };

      /** 
	   * Marks all location as important for storage.
	   * 
	   */
      struct MarkAllStates : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            for (auto& l : F->getCFA().getLocations()) {
              l->getInfo().template set<MiniMC::Model::Attributes::NeededStore>();
            }
          }
          return true;
        }
      };
    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
