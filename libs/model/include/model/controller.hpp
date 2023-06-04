#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "model/cfg.hpp"
#include "support/feedback.hpp"


#include <functional>
#include <vector>

namespace MiniMC {
  namespace Model {
    
    //Helper class for running modifications on a Program
    class Controller {
    public:
      Controller (MiniMC::Model::Program&& p) : prgm(std::move(p)) {
	lowerPhi ();
      }
      
      bool typecheck (MiniMC::Support::Messager mess = MiniMC::Support::Messager{});

      void lowerPhi ();
      void makeMemNonDet ();
      void createAssertViolateLocations ();
      void inlineFunctions (std::size_t);
      void unrollLoops (std::size_t);
      void expandNonDeterministic (MiniMC::Support::Messager mess = MiniMC::Support::Messager{});
      void onlyOneMemAccessPerEdge ();

      auto& getProgram () const {return prgm;}
      
    private:
      MiniMC::Model::Program prgm;
    };
  }
}


#endif
