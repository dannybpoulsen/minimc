#ifndef _CFG__
#define _CFG__

#include <memory>
#include <vector>
#include <gsl/pointers>

#include "model/instructions.hpp"
#include "model/variables.hpp"

namespace MiniMC {
  namespace Model {
    class Edge;    
    using Edge_ptr = std::shared_ptr<Edge>;
    class Location : public std::enable_shared_from_this<Location>{
    public:
      using edge_iterator = std::vector<gsl::not_null<Edge_ptr>>::const_iterator;
      
      Location (const std::string& n) : name(n) {}
      auto& getEdges () const {return edges;}
      void addEdge (gsl::not_null<Edge_ptr> e) {edges.push_back(e);}
      edge_iterator ebegin () const {return edges.begin();}
      edge_iterator eend () const {return edges.end();}
      auto& getName () const {return name;}
      
    private:
      std::vector<gsl::not_null<Edge_ptr>> edges;
      std::string name;
    };

    using Location_ptr = std::shared_ptr<Location>;
    
    class Instruction;
    class Edge {
    public:
      Edge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, const std::vector<Instruction>& inst, const Value_ptr& val) : instructions(inst),from(from),to(to),value(val) {}
      auto& getInstructions () const {return instructions;}
      auto getFrom () const {return from;}
      auto getTo () const {return to;}
      auto getGuard () const {return value;}
    private:
      std::vector<Instruction> instructions;
      gsl::not_null<Location_ptr> from;
      gsl::not_null<Location_ptr> to;
      Value_ptr value;
    };

    
    
	
    class CFG {
    public:
	  
      gsl::not_null<Location_ptr> makeLocation (const std::string& name) {
		locations.emplace_back (new Location (name));
		return locations.back();
      }
	  
      gsl::not_null<Edge_ptr> makeEdge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, const std::vector<Instruction>& inst, const Value_ptr& guard) {
	edges.emplace_back (new Edge (from,to,inst,guard));
	from->addEdge (edges.back());
	return edges.back();
      }

      gsl::not_null<Location_ptr> getInitialLocation () {
	assert(initial);
	return initial;
      }
	  
      void setInitial (gsl::not_null<Location_ptr> loc) {
	initial = loc.get();
      }
	  
    private:
      std::vector<Location_ptr>locations;
      std::vector<Edge_ptr> edges;
      Location_ptr initial = nullptr;;
    };

    using CFG_ptr = std::shared_ptr<CFG>;

    class Function {
    public:
      Function (std::size_t id, 
		const std::string& name,
		const std::vector<gsl::not_null<Variable_ptr>>& params,
		const VariableStackDescr_ptr& variableStackDescr,
		const gsl::not_null<CFG_ptr> cfg) : name(name),
						    parameters(params),
						    variableStackDescr(variableStackDescr),
						    cfg(cfg),
						    id(id) {}
      auto& getName() const {return name;}
      auto& getParameters () const {return parameters;}
      auto& getVariableStackDescr () const {return variableStackDescr;}
      auto& getCFG () const {return cfg;}
      auto& getID () const {return id;}
    private:
      std::string name;
      std::vector<gsl::not_null<Variable_ptr>> parameters;
      VariableStackDescr_ptr variableStackDescr;
      gsl::not_null<CFG_ptr> cfg;
      std::size_t id;
    };
    
    using Function_ptr = std::shared_ptr<Function>;

    class Program {
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
	return functions.back();
      }

      auto& getFunctions  () const {return functions;}
      void addEntryPoint (gsl::not_null<Function_ptr>& func) {
	entrypoints.push_back(func.get());
      }

      auto& getEntryPoints () const {return entrypoints;}
      
      gsl::not_null<VariableStackDescr_ptr> makeVariableStack () {
	return std::make_shared<VariableStackDescr> (stacks++); 
      }
      
    private:
      std::vector<Function_ptr> functions;
      VariableStackDescr_ptr globals;
      std::vector<Function_ptr> entrypoints;
      std::size_t stacks = 0;
	  
	};

	using Program_ptr = std::unique_ptr<Program>;
    
  }
}

#endif

