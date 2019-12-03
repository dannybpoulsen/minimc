#ifndef _CFG__
#define _CFG__

#include <memory>
#include <vector>
#include <algorithm>
#include <gsl/pointers>

#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/types.hpp"

namespace MiniMC {
  namespace Model {

    using line_loc = std::size_t;
    
    struct SourceLocation {
      SourceLocation (const std::string& filename, line_loc loc) : filename(filename),loc(loc) {}
      std::string filename;
      line_loc loc;
    };

    using SourceLocation_ptr = std::shared_ptr<SourceLocation>;
    
    class Edge;    
    using Edge_ptr = std::shared_ptr<Edge>;
    class Location : public std::enable_shared_from_this<Location>{
    public:
      using edge_iterator = std::vector<Edge_ptr>::const_iterator;
      
      Location (const std::string& n) : name(n) {}
      auto& getEdges () const {return edges;}
      void addEdge (gsl::not_null<Edge_ptr> e) {edges.push_back(e.get());}
      edge_iterator ebegin () const {return edges.begin();}
      edge_iterator eend () const {return edges.end();}
      auto& getName () const {return name;}
      void removeEdge (const Edge_ptr& e) {
	auto it = std::find (edges.begin(),edges.end(),e);
	if (it != edges.end()) {
	  edges.erase (it);
	}
      }

      bool hasSourceLocation  () const {return sourceloc.get();}
      gsl::not_null<SourceLocation_ptr> getSourceLoc () const {return sourceloc;} 
      void setSourceLoc (const SourceLocation_ptr& ptr) {sourceloc = ptr;} 
    private:
      std::vector<Edge_ptr> edges;
      std::string name;
      SourceLocation_ptr sourceloc = nullptr;
    };

    
    class Program;
    using Location_ptr = std::shared_ptr<Location>;
    using Program_ptr = std::shared_ptr<Program>;
    class Instruction;
    class Edge {
    public:
      Edge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, const std::vector<Instruction>& inst, const Value_ptr& val,bool neg = false) : instructions(inst),
																			     from(from),
																			     to(to),
																			     value(val),
																			     negGuard(neg) {}
      auto& getInstructions () const {return instructions;}
      auto& getInstructions ()  {return instructions;}
      auto getFrom () const {return from;}
      auto getTo () const {return to;}
      auto getGuard () const {return value;}
      auto negatedGuard () const {return  negGuard;}
      auto& getProgram() const {return prgm;}
      void setProgram (const Program_ptr& p) {prgm = p;} 
    private:
      std::vector<Instruction> instructions;
      gsl::not_null<Location_ptr> from;
      gsl::not_null<Location_ptr> to;
      Value_ptr value;
      bool negGuard;
      Program_ptr prgm;
    };

    
    inline std::ostream& operator<< (std::ostream& os, const Edge& e) {
      return os << e.getInstructions ();
    }
	
    class CFG {
    public:
	  
      gsl::not_null<Location_ptr> makeLocation (const std::string& name) {
	locations.emplace_back (new Location (name));
	return locations.back();
      }
	  
      gsl::not_null<Edge_ptr> makeEdge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, const std::vector<Instruction>& inst, const Value_ptr& guard, Program_ptr& p, bool neg = false) {
	edges.emplace_back (new Edge (from,to,inst,guard,neg));
	from->addEdge (edges.back());
	edges.back()->setProgram(p);
	return edges.back();
      }

      gsl::not_null<Location_ptr> getInitialLocation () {
	assert(initial);
	return initial;
      }
	  
      void setInitial (gsl::not_null<Location_ptr> loc) {
	initial = loc.get();
      }

      void deleteEdge (const Edge_ptr& edge) {
	auto it = std::find (edges.begin(),edges.end(),edge);
	if (it != edges.end()) {
	  edges.erase (it);
	}
	edge->getTo ()->removeEdge (edge);
      }

      auto& getLocations () const {return locations;}
      auto& getLocations ()  {return locations;}
      auto& getEdges () {return edges;}
    private:
      std::vector<Location_ptr>locations;
      std::vector<Edge_ptr> edges;
      Location_ptr initial = nullptr;;
    };

    using CFG_ptr = std::shared_ptr<CFG>;

    class Function : public std::enable_shared_from_this<Function> {
    public:
      Function (MiniMC::func_t id, 
		const std::string& name,
		const std::vector<gsl::not_null<Variable_ptr>>& params,
		const VariableStackDescr_ptr& variableStackDescr,
		const gsl::not_null<CFG_ptr> cfg) : name(name),
						    parameters(params),
						    variableStackDescr(variableStackDescr),
						    cfg(cfg),
						    id(id) {
	for (auto& e : cfg->getEdges ()) {
	  for (auto& l : e->getInstructions ())
	    l.setFunction (std::shared_ptr<Function> (this));
	}
	
      }
      auto& getName() const {return name;}
      auto& getParameters () const {return parameters;}
      auto& getVariableStackDescr () const {return variableStackDescr;}
      auto& getCFG () const {return cfg;}
      auto& getID () const {return id;}
      auto& getPrgm () const {return prgm;}
      void setPrgm (const Program_ptr& prgm ) {this->prgm = prgm;}
    private:
      std::string name;
      std::vector<gsl::not_null<Variable_ptr>> parameters;
      VariableStackDescr_ptr variableStackDescr;
      gsl::not_null<CFG_ptr> cfg;
      MiniMC::func_t id;
      Program_ptr prgm;
    };
    
    using Function_ptr = std::shared_ptr<Function>;

    class Program  : public std::enable_shared_from_this<Program>{
    public:
      Program ()  {
	globals = makeVariableStack().get();
      }
      gsl::not_null<VariableStackDescr_ptr> getGlobals () const { return globals;}
      gsl::not_null<Function_ptr>  addFunction (const std::string& name,
			const std::vector<gsl::not_null<Variable_ptr>>& params,
			const VariableStackDescr_ptr& variableStackDescr,
			const gsl::not_null<CFG_ptr> cfg) {
	functions.push_back (std::make_shared<Function> (functions.size(),name,params,variableStackDescr,cfg));
	functions.back()->setPrgm (this->shared_from_this ());
	return functions.back();
      }

      auto& getFunctions  () const {return functions;}
      void addEntryPoint (const gsl::not_null<Function_ptr>& func) {
	entrypoints.push_back(func.get());
      }
      
      Function_ptr getFunction (MiniMC::func_t id) const {
	return functions.at(id);
      }
      
      auto& getEntryPoints () const {return entrypoints;}

      bool hasEntryPoints () const {return entrypoints.size();}
      gsl::not_null<VariableStackDescr_ptr> makeVariableStack () {
	return std::make_shared<VariableStackDescr> (); 
      }

      auto makeSourceLocation (const std::string& n, line_loc l) const {
	return std::make_shared<SourceLocation> (n,l);
      }
      
    private:
      std::vector<Function_ptr> functions;
      VariableStackDescr_ptr globals;
      std::vector<Function_ptr> entrypoints;
      std::size_t stacks = 0;
      
    };
    
  }
}

#endif

