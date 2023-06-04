#include "model/cfg.hpp"
#include "model/modifications/constantfolding.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/insertboolcasts.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/removephi.hpp"
#include "model/modifications/replacesub.hpp"
#include "model/modifications/splitasserts.hpp"
#include "model/modifications/expandnondet.hpp"

#include "model/checkers/typechecker.hpp"

#include "model/controller.hpp"

namespace MiniMC {
  namespace Model {
    bool Controller::typecheck (MiniMC::Support::Messager mess) {
      return MiniMC::Model::Checkers::TypeChecker{prgm}.Check (mess);
    }
    
    void Controller::lowerPhi (){
      MiniMC::Model::Modifications::LowerPhi{}.run (prgm);
    }

    void Controller::createAssertViolateLocations (){
      MiniMC::Model::Modifications::SplitAsserts{}.run (prgm);
    }
    
    void Controller::inlineFunctions (std::size_t depth){
      for (auto& func : prgm.getEntryPoints ()) {
	MiniMC::Model::Modifications::InlineFunctions{prgm}.runFunction (func,depth);
      }
    }
    
    void Controller::unrollLoops (std::size_t iterations){
      for (auto& func : prgm.getFunctions ()) 
	MiniMC::Model::Modifications::UnrollLoops{}.runFunction (func,iterations);
    }
    
    
    void Controller::expandNonDeterministic (MiniMC::Support::Messager mess){
      MiniMC::Model::Modifications::expandNonDet (prgm,mess);
      
    }
   
  }
} // namespace MiniMC
