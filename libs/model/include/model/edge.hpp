#ifndef __EDGE__
#define __EDGE__

#include <memory>
#include <gsl/pointers>

#include "model/location.hpp"

namespace MiniMC {
  namespace Model {

	class Program;
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
      using iterator = std::vector<Instruction>::iterator;
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

      template<class Iterator>
      auto replaceInstructionBySeq (iterator repl, Iterator beg, Iterator end) {
		return instr.insert(erase (repl),beg,end);
      }
      
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
      Edge (gsl::not_null<Location_ptr> from, gsl::not_null<Location_ptr> to, const Program_wptr& prgm) : 
		from(from.get()),
		to(to.get()),
		prgm(prgm)
	  {
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
      
      //void setProgram (const Program_ptr& p) {prgm = p;} 
      
      void copyAttributesFrom (const Edge& e) {
		if (e.hasAttribute<AttributeType::Guard> ()){
		  this->setAttribute<AttributeType::Guard> (e.getAttribute<AttributeType::Guard> ());
		}

		if (e.hasAttribute<AttributeType::Instructions> ()){
		  this->setAttribute<AttributeType::Instructions> (e.getAttribute<AttributeType::Instructions> ());
		}
	
      }
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

  }
}


#endif
