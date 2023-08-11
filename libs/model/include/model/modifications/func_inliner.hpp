/**
 * @file   func_inliner.hpp
 * @date   Mon Apr 20 17:04:05 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _funcinliner__
#define _funcinliner__

#include "model/cfg.hpp"
#include "model/modifications/modifications.hpp"
namespace MiniMC {
  namespace Model {
    namespace Modifications {

      struct InlineFunctions  : public ProgramModifier {
        InlineFunctions(std::size_t depth) : depth(depth) {}
	
	MiniMC::Model::Program operator() (MiniMC::Model::Program&& program) {
	  prgm = &program;
	  for (auto& func : prgm->getEntryPoints ())
	    runFunction (func,depth);
	  prgm = nullptr;
	  return program;
	}
	
      private:
	bool runFunction(const MiniMC::Model::Function_ptr& F,std::size_t depth);
        
	MiniMC::Model::Program* prgm{nullptr};
	std::size_t depth;
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
