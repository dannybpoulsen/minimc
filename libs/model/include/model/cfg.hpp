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

      bool isErrorLocation () const {
	return isError;
      }

      void setError ()  {
	isError = true;
      }
      
      bool hasSourceLocation  () const {return sourceloc.get();}
      gsl::not_null<SourceLocation_ptr> getSourceLoc () const {return sourceloc;} 
      void setSourceLoc (const SourceLocation_ptr& ptr) {sourceloc = ptr;} 
    private:
      std::vector<Edge_ptr> edges;
      std::string name;
      SourceLocation_ptr sourceloc = nullptr;
      bool isError = false;
    };

    
    class Program;
    using Location_ptr = std::shared_ptr<Location>;
    using Program_ptr = std::shared_ptr<Program>;
    class Instruction;

    enum class AttributeType {
			      Instructions,
			      Guard
    };

    template<AttributeType>
    struct AttributeValueType {
      using ValType = bool;
    };

    struct Guard {
      Guard () {}
      Guard (const Value_ptr& g, bool negate) : guard(g),negate(negate) {}
      Value_ptr guard = nullptr;
      bool negate = false;
    };
	
    struct InstructionStream {
      InstructionStream () : isPhi(false) {}
      InstructionStream (const std::vector<Instruction>& i, bool isPhi = false) : instr(i),
										  isPhi(isPhi) {
	assert(instr.size());
      }
      InstructionStream (const InstructionStream& str) : instr(str.instr),isPhi(str.isPhi) {}
      auto begin () const {return instr.begin();}
      auto end () const {return instr.end();}
      auto begin ()  {return instr.begin();}
      auto end ()  {return instr.end();}
      auto& last () {assert(instr.size());return instr.back();}
      std::vector<Instruction> instr;
      bool isPhi = false;;
    };
    
    template<>
    struct AttributeValueType<AttributeType::Instructions> {
      using ValType = InstructionStream;
    };
    
    template<>
    struct AttributeValueType<AttributeType::Guard> {
      using ValType = Guard;
    };
    
    
    class IEdgeAttributes {
    public:
      virtual AttributeType getType () = 0;
      virtual bool is (AttributeType i) const = 0;
    };

    template<AttributeType k>
    class EdgeAttributesMixin {
    public:
      using ValType = typename AttributeValueType<k>::ValType;
      void setValue (const ValType& v) {
		assert(!isSet());
		val = v;
		is_set = true;
      }
	  
      auto& getValue () const {return val;}
      auto& getValue () {return val;}
      
	  
      bool isSet () const {
		return is_set;
      }
    private:
      ValType val;
      bool is_set = false;
    };
	
    
    
    

    class Edge : private EdgeAttributesMixin<AttributeType::Instructions>,
				 private EdgeAttributesMixin<AttributeType::Guard>
    {
    public:
      Edge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to) : 
		from(from),
		to(to)
		//value(val),
		//negGuard(neg) {
      {
		//if(val)
		//  this->template setAttribute<AttributeType::Guard> (Guard(val,neg));
      }
	  
      template<AttributeType k>
      void setAttribute (const typename AttributeValueType<k>::ValType& inp) {
	static_cast<EdgeAttributesMixin<k>*>(this) ->setValue (inp);
      }
      
      template<AttributeType k>
      auto& getAttribute () {
	return static_cast<EdgeAttributesMixin<k>*>(this) ->getValue ();
      }
	  
      template<AttributeType k>
      auto& getAttribute () const {
	return static_cast<const EdgeAttributesMixin<k>*>(this) ->getValue ();
      }
      
      template<AttributeType k>
      auto hasAttribute () const {
	return static_cast<const EdgeAttributesMixin<k>*>(this) ->isSet ();
      }
      
      
      auto getFrom () const {return from;}
      auto getTo () const {return to;}
      auto& getProgram() const {return prgm;}
      void setProgram (const Program_ptr& p) {prgm = p;} 
    private:
      gsl::not_null<Location_ptr> from;
      gsl::not_null<Location_ptr> to;
      Value_ptr value;
      Program_ptr prgm;
    };

    
    inline std::ostream& operator<< (std::ostream& os, const Edge& e) {
	  if (e.hasAttribute<AttributeType::Instructions> ()) {
		os <<e.getAttribute<AttributeType::Instructions> ().instr;
	  }
	  //return os << e.getInstructions ();
      return os;
    }
	
    class CFG {
    public:
	  
      gsl::not_null<Location_ptr> makeLocation (const std::string& name) {
	locations.emplace_back (new Location (name));
	return locations.back();
      }
	  
      gsl::not_null<Edge_ptr> makeEdge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, Program_ptr& p) {
	edges.emplace_back (new Edge (from,to));
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
	  if (e->hasAttribute<AttributeType::Instructions> ()) 
	    for (auto& l : e->getAttribute<AttributeType::Instructions> ())
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

      bool  functionExists (MiniMC::func_t id) const {
	return id < functions.size();
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

