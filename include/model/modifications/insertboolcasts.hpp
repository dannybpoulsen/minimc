/**
 * @file   insertboolcasts.hpp
 * @date   Mon Apr 20 17:03:54 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _INSERTBOOLCASTS__
#define _INSERTBOOLCASTS__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      /**
	   * Transform ZExt (SEx) Instructions into BoolZExt (BoolSExt) when the castee is a boolean type. 
	   * This is useful since the LLVM Loader will not distinguish between integers and booleans thus ZExt/SExt 
	   * is inserted irrespective of the type when an extension is needed.
	   * The analysis algorithms may distinguish however, so to avoid
	   * a runtime switch we want to convert them before analysis
	   * starts. 
	   */
      struct InsertBoolCasts : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	
        virtual bool runFunction(const MiniMC::Model::Function_ptr& F);
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            runFunction(F);
          }
          return true;
        }
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
