#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "model/cfg.hpp"

namespace MiniMC {
  namespace Model {
    //Helper class for running modifications on a Program
    class Controller {
    public:
      Controller (const MiniMC::Model::Program& p) : prgm(std::make_shared<Program> (p)) {
	lowerPhi ();
      }
      Controller (MiniMC::Model::Program&& p) : prgm(std::make_shared<Program> (std::move(p))) {}
      
      bool typecheck ();
      bool structuralcheck ();
      
      void lowerPhi ();
      void makeMemNonDet ();
      void createAssertViolateLocations ();
      void inlineFunctions (std::size_t, const MiniMC::Model::Function_ptr&);
      void unrollLoops (std::size_t, const MiniMC::Model::Function_ptr&);
      void boolCasts ();
      void splitAtCMP ();
      void lowerGuards ();
      void expandNonDet ();
      void simplifyCFA ();
      void onlyOneMemAccessPerEdge ();

      void markLoopLocations ();
      void makeLoopAllLocations ();
      void addEntryPoint (std::string& name, std::vector<MiniMC::Model::Value_ptr>&&);
      auto& getProgram () const {return prgm;}
      
    private:
      MiniMC::Model::Program_ptr prgm;
    };
  }
}


#endif
