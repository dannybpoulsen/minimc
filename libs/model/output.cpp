#include "model/output.hpp"
#include "model/cfg.hpp"

#include <ostream>
#include <list>
#include <unordered_set>


namespace MiniMC {
  namespace Model {
    void writeEdge (std::ostream& os, const MiniMC::Model::Edge& edge) {
      if (edge.getInstructions () ){
	for (auto& i : edge.getInstructions () ) {
	  i.output (os << "      ") << "\n";
	}
      }
    }
    
    void writeCFA (std::ostream& os, const MiniMC::Model::CFA& cfa) {
      os << "  .cfa" << "\n";
      std::unordered_set<MiniMC::Model::Location_ptr> seen;
      std::list<MiniMC::Model::Location_ptr> waiting;
      seen.insert ( cfa.getInitialLocation ());
      waiting.push_back (cfa.getInitialLocation ());
      while (waiting.size ()) {
	auto cur = waiting.back ();
	waiting.pop_back ();
	os << "    " << "BB" << cur->getID ()<< " " << " {" << cur->getInfo().getName () <<"}\n";
        if(cur->getInfo().is<Attributes::AssertViolated>()){
          os << "    @AssertViolated\n";
        }
	os << "    [" << "\n";
	auto it = cur->ebegin ();
	auto end = cur->eend ();
	for (; it != end; ++it) {
	  writeEdge (os, **it);
	  os << "      ->" << "BB" << (*it)->getTo ()->getID () << "\n";
	  if (!seen.count ((*it)->getTo ())) {
	    seen.insert ((*it)->getTo ());
	    waiting.push_back ((*it)->getTo ());
	  }
	}
	os << "    ]" << "\n";
	
      }
      
    }
    
    void writeFunction (std::ostream& os, const MiniMC::Model::Function& F) {
      os << "## " << F.getSymbol () <<"\n";
      os << "  .registers" << "\n";
      for (auto& reg : F.getRegisterDescr ().getRegisters()) {
	os << "    " << *reg << "\n";
      }
      os << "  .parameters" << "\n";
      for (auto& reg : F.getParameters ()) {
	os << "    " << reg->getName () << "\n";
      }
      os << "  .returns" <<  "\n";
      os << "    " << *F.getReturnType () <<"\n";
      writeCFA (os,F.getCFA ());
    }
    
    void writeProgram (std::ostream& os, const MiniMC::Model::Program& p) {
      os << "# Functions" << "\n";
      for (auto& F : p.getFunctions ()) {
	writeFunction (os,*F);
      }
      
      os << "# Entrypoints" <<"\n";
      for (auto& F : p.getEntryPoints ()) {
	os << "  " << F->getSymbol () << "\n";
      }

      os << "# Heap" <<"\n";
      for (auto b : p.getHeapLayout ()) {
	os << b.baseobj << " : " << b.size << "\n"; 
      }
      
      os << "# Initialiser" <<"\n";
      for (auto i : p.getInitialiser ()) {
	i.output (os <<"  ") << "\n";
      }
      
      
    }
  }
}

