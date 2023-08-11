#include "model/cfg.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/removephi.hpp"
#include "model/modifications/splitasserts.hpp"
#include "model/modifications/expandnondet.hpp"

#include "model/checkers/typechecker.hpp"

#include "model/controller.hpp"

namespace MiniMC {
  namespace Model {
    
    void Controller::lowerPhi (){
      prgm = MiniMC::Model::Modifications::LowerPhi{} (std::move(prgm));
    }
    
    void Controller::createAssertViolateLocations (){
      prgm = MiniMC::Model::Modifications::SplitAsserts{} (std::move(prgm));
    }
    
    void Controller::inlineFunctions (std::size_t depth){
      prgm = MiniMC::Model::Modifications::InlineFunctions{depth} (std::move(prgm));
	
    }
    
    void Controller::unrollLoops (std::size_t iterations){
      prgm = MiniMC::Model::Modifications::UnrollLoops{iterations}(std::move(prgm));
    }
    
    
    void Controller::expandNonDeterministic (MiniMC::Support::Messager mess){
      prgm = MiniMC::Model::Modifications::NonDetExpander{mess}(std::move(prgm));
      //MiniMC::Model::Modifications::expandNonDet (prgm,mess);
      
    }
   
  }
} // namespace MiniMC
