#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "model/cfg.hpp"

#include <functional>
#include <vector>

namespace MiniMC {
  namespace Model {
    
    //Helper class for running modifications on a Program
    class Controller {
    public:
      Controller (const MiniMC::Model::Program& p) : prgm(std::make_shared<Program> (p)) {
	lowerPhi ();
      }
      
      bool typecheck ();
      bool structuralcheck ();
      
      void lowerPhi ();
      void makeMemNonDet ();
      void createAssertViolateLocations ();
      void inlineFunctions (std::size_t);
      void unrollLoops (std::size_t);
      void boolCasts ();
      void splitAtCMP ();
      void expandNonDeterministic ();
      void onlyOneMemAccessPerEdge ();

      auto& getProgram () const {return prgm;}
      
    private:
      MiniMC::Model::Program_ptr prgm;
    };
  }
}


#endif
