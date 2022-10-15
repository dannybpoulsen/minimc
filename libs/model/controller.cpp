#include "model/cfg.hpp"
#include "model/modifications/constantfolding.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/insertboolcasts.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/removephi.hpp"
#include "model/modifications/replacesub.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/splitasserts.hpp"
#include "model/modifications/splitcmps.hpp"
#include "model/modifications/expandnondet.hpp"

#include "model/checkers/structure.hpp"
#include "model/checkers/typechecker.hpp"

#include "model/controller.hpp"

namespace MiniMC {
  namespace Model {
    bool Controller::typecheck () {
      return MiniMC::Model::Checkers::TypeChecker{}.run (*prgm);
    }
    
    bool Controller::structuralcheck () {
      return MiniMC::Model::Checkers::StructureChecker{}.run (*prgm);
    }

    void Controller::lowerPhi (){
      MiniMC::Model::Modifications::LowerPhi{}.run (*prgm);
    }

    void Controller::boolCasts (){
      MiniMC::Model::Modifications::InsertBoolCasts{}.run (*prgm);
    }

    void Controller::createAssertViolateLocations (){
      MiniMC::Model::Modifications::SplitAsserts{}.run (*prgm);
    }
    
    void Controller::inlineFunctions (std::size_t depth, const MiniMC::Model::Function_ptr& func){
      MiniMC::Model::Modifications::InlineFunctions{*prgm}.runFunction (func,depth);
    }
    
    void Controller::unrollLoops (std::size_t iterations,  const MiniMC::Model::Function_ptr& func){
      MiniMC::Model::Modifications::UnrollLoops{}.runFunction (func,iterations);
    }
    
    
    void Controller::expandNonDeterministic (){
      MiniMC::Model::Modifications::expandNonDet (*prgm);
    }

      
    void Controller::addEntryPoint (std::string& name, std::vector<MiniMC::Model::Value_ptr>&& params) {
      auto func = prgm->getFunction(name);
      auto entry = createEntry(*prgm, func,std::move(params));
      prgm->addEntryPoint(entry->getSymbol().getName());
    }
    
  }
} // namespace MiniMC
