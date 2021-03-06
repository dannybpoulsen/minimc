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
#include "support/types.hpp"

namespace MiniMC {
  namespace Model {

    using line_loc = std::size_t;
    using col_loc = std::size_t;
	
    struct SourceLocation {
      SourceLocation (const std::string& filename, line_loc loc,col_loc col) : filename(filename),loc(loc),col(col) {}
      std::string filename;
      line_loc loc;
      col_loc col;
    };

    using SourceLocation_ptr = std::shared_ptr<SourceLocation>;
    
    class Edge;    
	using Edge_ptr = std::shared_ptr<Edge>;
	using Edge_wptr = std::weak_ptr<Edge>;

	template<class Obj, class BaseIterator>
	class SmartIterator {
	public:
	  SmartIterator (BaseIterator iter) : iter(iter) {}
	  bool operator== (const SmartIterator<Obj,BaseIterator>& oth) const  {
		return oth.iter == iter;
	  }

	  bool operator!= (const SmartIterator<Obj,BaseIterator>& oth) const {
		return oth.iter != iter;
	  }

	  Obj operator-> () {
		return (*iter).lock();
	  }

	  Obj operator* () {
		return (*iter).lock();
	  }

	  void operator++ () {
		++iter;
	  }
	  
	private:
	  BaseIterator iter;
	};

	/** 
	 * Location in a functions CFG
	 * Locations can be assigned different attributes which can affect exploration algorithms, or the modification algorihtms on the CFG. 
	 */
    class Location : public std::enable_shared_from_this<Location>{
    public:
	  friend class CFG;
	  friend class Edge;
      using AttrType = char;

	  /**
	   * The possible attributes that can be assigned locations
	   *
	   */
      enum class Attributes  : AttrType {
										 AssertViolated = 1, /**< Indicates an assert was violated */
										 NeededStore = 2, /**< Indicates this location is part of loop, and must be stored for guaranteeing termination*/
										 CallPlace = 4, /**< Indicates a call takes place on an edge leaving this location */
										 AssumptionPlace = 8 /**< Indicates an assumption is made on an edge leaving this location  */
      };
      
      using edge_iterator = SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator>;
      
      Location (const std::string& n, MiniMC::offset_t id) : name(n),id(id) {}
      
      void addEdge (gsl::not_null<Edge_ptr> e) {edges.push_back(e.get());}
      void addIncomingEdge (gsl::not_null<Edge_ptr> e) {incomingEdges.push_back(e.get());}
      /** 
	   *
	   * @return begin iterator for outgoing edges
	   */
      edge_iterator ebegin () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (edges.begin());}

	  /** 
	   *
	   * @return end iterator for outgoing edges
	   */
	  edge_iterator eend () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (edges.end());}

	  /** 
	   *
	   * @return begin iterator for incoming edges
	   */
	  edge_iterator iebegin () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (incomingEdges.begin());}

	  /** 
	   *
	   * @return end iterator for incoming edges
	   */
	  edge_iterator ieend () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (incomingEdges.end());}

	  /** 
	   * Check if this location has outoing edges
	   *
	   * @return true if it has outgoing edges, false if not
	   */
      bool hasOutgoingEdge () const {
		return edges.size();
      }
      
      auto& getName () const {return name;}

	  

	  /** 
	   * Count the number of incoming edges
	   *
	   *
	   * @return number of incoming edges
	   */
      auto nbIncomingEdges () const  {
		return incomingEdges.size();
      }

	  /** 
	   * Check  if this location has Attributes \p i set 
	   *
	   *
	   * @return true if \p i  is set false otherwise
	   */
      template<Attributes i>
      bool is () {
		return static_cast<AttrType> (i) & flags;
      }

	  /** 
	   * Set attribute \p i.
	   *
	   */
      template<Attributes i>
      void set () {
		flags |= static_cast<AttrType> (i);
      }

	  /** 
	   * Remove attribute \p i from this Location.
	   *
	   */
      template<Attributes i>
      void unset () {
		flags &= ~static_cast<AttrType> (i);
      }

	  AttrType getAttributesFlags () const {return flags;}
	  void setAttributesFlags (AttrType t)  {flags = t;}
	  
	  
      auto getID () const {return id;}
      
      bool hasSourceLocation  () const {return sourceloc.get();}
      gsl::not_null<SourceLocation_ptr> getSourceLoc () const {return sourceloc;} 
      void setSourceLoc (const SourceLocation_ptr& ptr) {sourceloc = ptr;} 

	protected:
	  
	  /** 
	   * Search for an edge \p e. If found delete it from the outgoing edges.
	   * Notice \p e is searched for using pointer equality.  
	   *
	   * @param e The edge to search for
	   */
      void removeEdge (const Edge_ptr e) {
		auto it = std::find_if (edges.begin(),edges.end(),
								[&e](const Edge_wptr& ptr1) {
								  return ptr1.lock() == e;
								});
		assert(it != edges.end());
		edges.erase (it);
      }

	  /** 
	   * Search for an edge \p e. If found delete it from the incoming  edges.
	   * Notice \p e is searched for using pointer equality.  
	   *
	   * @param e The edge to search for
	   */
      void removeIncomingEdge (const Edge_ptr e) {
		auto it = std::find_if (incomingEdges.begin(),incomingEdges.end(),
								[&e](const Edge_wptr& ptr1) {
								  return ptr1.lock() == e;
								});
		assert (it != incomingEdges.end());
		incomingEdges.erase (it);
		
      }
	  
	private:
      std::vector<Edge_wptr> edges;
      std::vector<Edge_wptr> incomingEdges;
      std::string name;
      SourceLocation_ptr sourceloc = nullptr;
      bool isError = false;
      AttrType flags = 0;
      MiniMC::offset_t id;
    };

    
    class Program;
    using Location_ptr = std::shared_ptr<Location>;
	using Location_wptr = std::weak_ptr<Location>;
    using Program_ptr = std::shared_ptr<Program>;
	using Program_wptr = std::weak_ptr<Program>;
	
	class Instruction;

	/** 
	 * Possible attributes that can be set on edges
	 *
	 */
    enum class AttributeType {
							  Instructions, /**<  A stream of instruction*/
							  Guard  /**<  A  guarding Value that has to be true when moving along the edge*/
    };
	
    template<AttributeType>
    struct AttributeValueType {
      using ValType = bool;
    };

	/**
	 * Structure for representing guards. 
	 */
    struct Guard {
      Guard () {}
      Guard (const Value_ptr& g, bool negate) : guard(g),negate(negate) {}
	  
	  Value_ptr guard = nullptr; ///< The guarding Value
	  bool negate = false; ///< whether the guarding value should be negated when evaluating if the guard is true 
    };

    inline std::ostream& operator<< (std::ostream& os, const Guard& g) {
      return os << "<< " <<(g.negate ? "!" : "") << *g.guard << " >>";
    }
	
	/**
	 * Structure representing InstructionStream on  edges.
	 * An InstructionStream is a sequence of instruction to be performed uninterrupted.
	 * If the isPhi is true, then the InstructionStream must be performed without the instructions affecting eachother (this is to 
	 * to reflect that some operations are atomic even within a single process - such as phi-nodes in SSA form).
	 */
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

      auto rbegin () const {return instr.rbegin();}
      auto rend () const {return instr.rend();}
      auto rbegin ()  {return instr.rbegin();}
      auto rend ()  {return instr.rend();}
	  
      auto& last () {assert(instr.size()); return instr.back();}

      auto back_inserter () {return std::back_inserter(instr);}
      
      template<class Iterator>
      auto erase( Iterator iter) {
		return instr.erase (iter);
      }
	  
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
      
      void unSet ()  {
		is_set = false;
      }
    private:
      ValType val;
      bool is_set = false;
    };
	
    
    
	
	
    class Edge : private EdgeAttributesMixin<AttributeType::Instructions>,
				 private EdgeAttributesMixin<AttributeType::Guard>,
				 public std::enable_shared_from_this<Edge> 
    {
    public:
      Edge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to) : 
		from(from.get()),
		to(to.get()) {
      }

	  Edge (const Edge& ) = default;
	  
      template<AttributeType k>
      void setAttribute (const typename AttributeValueType<k>::ValType& inp) {
		static_cast<EdgeAttributesMixin<k>*>(this) ->setValue (inp);
      }

      template<AttributeType k>
      void delAttribute () {
		static_cast<EdgeAttributesMixin<k>*>(this) ->unSet ();
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
      
      
      auto getFrom () const {return gsl::not_null<Location_ptr> (from.lock());}
      auto getTo () const {return gsl::not_null<Location_ptr> (to.lock());}

	  /** 
	   *  Set the to Location of this Edge. Also remove the edge
	   *  itself from the current to.
	   *
	   * @param t New target of the edge 
	   */
	  void setTo (gsl::not_null<Location_ptr> t) {
		to.lock()->removeIncomingEdge (this->shared_from_this());
		to = t.get();
		t->addIncomingEdge (this->shared_from_this());
      }
	  
      auto getProgram() const {return prgm.lock();}

	  void setProgram (const Program_ptr& p) {prgm = p;} 
    private:
      Location_wptr from;
      Location_wptr to;
      Value_ptr value;
      Program_wptr prgm;
    };

    
    inline std::ostream& operator<< (std::ostream& os, const Edge& e) {
      if (e.hasAttribute<AttributeType::Guard> ()) {
		os <<e.getAttribute<AttributeType::Guard> ();
      }
      else if (e.hasAttribute<AttributeType::Instructions> ()) {
		os <<e.getAttribute<AttributeType::Instructions> ().instr;
      }
      return os;
    }

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

      auto makeSourceLocation (const std::string& n, line_loc l, col_loc c) const {
		return std::make_shared<SourceLocation> (n,l,c);
      }

      auto& getConstantFactory () {return cfact;}
      auto& getTypeFactory () {return tfact;}

	  auto& getInitialisation () const {return initialiser;}
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

