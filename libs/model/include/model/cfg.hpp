
/**
 * @file   cfg.hpp
 * @date   Sun Apr 19 20:16:17 2020
 * 
 * 
 */
#ifndef _CFG__
#define _CFG__

#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <gsl/pointers>

#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "model/location.hpp"
#include "model/edge.hpp"
#include "support/types.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {        
    

	/**
	 *
	 * Representation of an Control Flow Automaton (despite the misleading name CFG). 
	 * The CFG is responsible for creating( and deleting) edges and
	 * locations of a function. It will also make sure that  the
	 * incoming/outgoing edges of locations are properly update when
	 * deleting edges. This 
	 *
	 */
    class CFG {
    public:
      CFG () {}
	  
      gsl::not_null<Location_ptr> makeLocation (const std::string& name) {
		locations.emplace_back (new Location (name,locations.size()));
		return locations.back();
      }

	  /** 
	   * Make a new edge 
	   *
	   * @param from source of the edge
	   * @param to target of the edge
	   * @param p the program that the edge is associated to.
	   *
	   * @return 
	   */
      gsl::not_null<Edge_ptr> makeEdge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, const Program_ptr& p) {
		edges.emplace_back (new Edge (from,to));
		to->addIncomingEdge (edges.back ());
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

	  /** 
	   * Delete \p edge from this CFG. Update also the
	   * incoming/outgoing edges of the target/source of \p edge. 
	   *
	   * @param edge The edge to delete
	   */
      void deleteEdge (const Edge_ptr& edge) {
		edge->getFrom ()->removeEdge (edge);
		edge->getTo ()->removeIncomingEdge (edge);
		
		
		auto it = std::find (edges.begin(),edges.end(),edge);
		if (it != edges.end()) {
		  edges.erase (it);
		}
      }
	  
      void deleteLocation (const Location_ptr& location) {
		MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
		auto insert = wlist.inserter();
		std::for_each (location->ebegin(),location->eend(),[&](const auto& e) {insert = e;});
		std::for_each (location->iebegin(),location->ieend(),[&](const auto& e) {insert = e;});
	
	
	
		std::for_each (wlist.begin(),wlist.end(), [&](const auto& e) {this->deleteEdge (e);});
	

		//edge->getFrom ()->removeEdge (edge);
		//edge->getTo ()->removeIncomingEdge (edge);
	
	
		auto it = std::find (locations.begin(),locations.end(),location);
		if (it != locations.end()) {
		  locations.erase (it);
		}
      }
      
      auto& getLocations () const {return locations;}
      auto& getLocations ()  {return locations;}
      auto& getEdges () {return edges;}

	  //Check if locations and edges are consistent
	  bool isIncomingOutgoingConsistent () const {
		for (auto& e : edges) {
		  if (!e->getTo ()->isIncoming (e) || !e->getFrom ()->isOutgoing (e)) {
			return false;
		  }
		}
		return true;
		
	  }
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
				const gsl::not_null<Type_ptr> rtype,
				const VariableStackDescr_ptr& variableStackDescr,
				const gsl::not_null<CFG_ptr> cfg) : name(name),
													parameters(params),
													variableStackDescr(variableStackDescr),
													cfg(cfg),
													id(id),
													retType(rtype)
      {
		
      }

      void takeOwnsership () {
		auto wptr = std::shared_ptr<Function>( this, [](Function*){} ); 
		for (auto& e : cfg->getEdges ()) {
		  if (e->hasAttribute<AttributeType::Instructions> ()) 
			for (auto& l : e->getAttribute<AttributeType::Instructions> ()) {
			  l.setFunction (shared_from_this());
			}
		}
      }
	  
      auto& getName() const {return name;}
      auto& getParameters () const {return parameters;}
      auto& getVariableStackDescr () const {return variableStackDescr;}
      auto& getVariableStackDescr ()  {return variableStackDescr;}
      auto& getCFG () const {return cfg;}
      auto& getID () const {return id;}
      auto& getReturnType () {return retType;}
      gsl::not_null<Program_ptr> getPrgm () const {return prgm.lock();}
      void setPrgm (const Program_ptr& prgm ) {this->prgm = prgm;}
    private:
      std::string name;
      std::vector<gsl::not_null<Variable_ptr>> parameters;
      VariableStackDescr_ptr variableStackDescr;
      gsl::not_null<CFG_ptr> cfg;
      MiniMC::func_t id;
      Program_wptr prgm;
      Type_ptr retType;
    };
    
    using Function_ptr = std::shared_ptr<Function>;

	
    class Program  : public std::enable_shared_from_this<Program>{
    public:
      Program (const MiniMC::Model::TypeFactory_ptr& tfact,
			   const MiniMC::Model::ConstantFactory_ptr& cfact
			   ) : cfact(cfact), tfact(tfact)  {
		globals = makeVariableStack().get();
      }

	  gsl::not_null<VariableStackDescr_ptr> getGlobals () const { return globals;}
      gsl::not_null<Function_ptr>  addFunction (const std::string& name,
												const std::vector<gsl::not_null<Variable_ptr>>& params,
												const gsl::not_null<Type_ptr> retType,
												const VariableStackDescr_ptr& variableStackDescr,
												const gsl::not_null<CFG_ptr> cfg) {
		functions.push_back (std::make_shared<Function> (functions.size(),name,params,retType,variableStackDescr,cfg));
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

      bool  functionExists (MiniMC::func_t id) const {
		return id < functions.size();
      }
      
      auto& getEntryPoints () const {return entrypoints;}

      bool hasEntryPoints () const {return entrypoints.size();}
      gsl::not_null<VariableStackDescr_ptr> makeVariableStack () {
		return std::make_shared<VariableStackDescr> (); 
      }

      auto& getConstantFactory () {return cfact;}
      auto& getTypeFactory () {return tfact;}
	  
      const auto& getInitialisation () const {return initialiser;}
      void setInitialiser (const InstructionStream& instr) {initialiser = instr;}
      
    private:
      std::vector<Function_ptr> functions;
      VariableStackDescr_ptr globals;
      std::vector<Function_ptr> entrypoints;
      std::size_t stacks = 0;
      MiniMC::Model::ConstantFactory_ptr cfact;
      MiniMC::Model::TypeFactory_ptr tfact;
      InstructionStream initialiser;
    };
    
  }
}

namespace std {
  template<>
  struct hash<MiniMC::Model::Location> {
	std::size_t operator() (const MiniMC::Model::Location& loc) {return reinterpret_cast<size_t> (&loc);}
  };
}

#endif

