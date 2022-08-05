#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "model/cfg.hpp"

#include <functional>
#include <vector>

namespace MiniMC {
  namespace Model {
    using entry_creator = std::function<MiniMC::Model::Function_ptr(MiniMC::Model::Program&, MiniMC::Model::Function_ptr,std::vector<MiniMC::Model::Value_ptr>&&)>;
    
    //Helper class for running modifications on a Program
    class Controller {
    public:
      Controller (const MiniMC::Model::Program& p,entry_creator entry) : prgm(std::make_shared<Program> (p)),createEntry(entry) {
	lowerPhi ();
      }
      
      bool typecheck ();
      bool structuralcheck ();
      
      void lowerPhi ();
      void makeMemNonDet ();
      void createAssertViolateLocations ();
      void inlineFunctions (std::size_t, const MiniMC::Model::Function_ptr&);
      void unrollLoops (std::size_t, const MiniMC::Model::Function_ptr&);
      void boolCasts ();
      void splitAtCMP ();
      void expandNonDeterministic ();
      void onlyOneMemAccessPerEdge ();

      void addEntryPoint (std::string& name, std::vector<MiniMC::Model::Value_ptr>&&);
      auto& getProgram () const {return prgm;}
      
    private:
      MiniMC::Model::Program_ptr prgm;
      entry_creator createEntry;
    };
  }
}


#endif
