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
    class Location {
    public:
      Location (const std::string& n) : name(n) {}
      auto& getEdges () const {return edges;}
      void addEdge (gsl::not_null<Edge*> e) {edges.push_back(e);}
      
    private:
      std::vector<gsl::not_null<Edge*>> edges;
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

	using Edge_ptr = std::shared_ptr<Edge>;
    
	
    class CFG {
    public:
	  
      gsl::not_null<Location_ptr> makeLocation (const std::string& name) {
		locations.emplace_back (new Location (name));
		return locations.back();
      }
	  
      gsl::not_null<Edge_ptr> makeEdge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, const std::vector<Instruction>& inst, const Value_ptr& guard) {
		edges.emplace_back (new Edge (from,to,inst,guard));
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
	  Function (const std::string& name,
				const std::vector<gsl::not_null<Variable_ptr>>& params,
				const VariableStackDescr_ptr& variableStackDescr,
				const gsl::not_null<CFG_ptr> cfg) : name(name),
													parameters(params),
													variableStackDescr(variableStackDescr),
													cfg(cfg) {}
	  auto& getName() const {return name;}
	  auto& getParameters () const {return parameters;}
	  auto& getVariableStackDescr () const {return variableStackDescr;}
	  auto& getCFG () const {return cfg;}
	private:
	  std::string name;
	  std::vector<gsl::not_null<Variable_ptr>> parameters;
	  VariableStackDescr_ptr variableStackDescr;
	  gsl::not_null<CFG_ptr> cfg;
	};
    
  }
}

#endif

