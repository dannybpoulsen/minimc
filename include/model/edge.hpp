#ifndef __EDGE__
#define __EDGE__

#include <memory>
#include <ostream>
#include <utility>

#include "model/location.hpp"
#include "model/variables.hpp"
#include "model/instructions.hpp"

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
      Guard         /**<  A  guarding Value that has to be true when moving along the edge*/
    };

    template <AttributeType>
    struct AttributeValueType {
      using ValType = bool;
    };

    /**
	 * Structure for representing guards. 
	 */
    struct Guard {
      Guard() {}
      Guard(const Value_ptr& g, bool negate) : guard(g), negate(negate) {}

      Value_ptr guard = nullptr; ///< The guarding Value
      bool negate = false;       ///< whether the guarding value should be negated when evaluating if the guard is true
    };

    inline std::ostream& operator<<(std::ostream& os, const Guard& g) {
      return os << "<< " << (g.negate ? "!" : "") << *g.guard << " >>";
    }

    
    template <>
    struct AttributeValueType<AttributeType::Instructions> {
      using ValType = InstructionStream;
    };

    template <>
    struct AttributeValueType<AttributeType::Guard> {
      using ValType = Guard;
    };

    class IEdgeAttributes {
    public:
      virtual AttributeType getType() = 0;
      virtual bool is(AttributeType i) const = 0;
    };

    template <AttributeType k>
    class EdgeAttributesMixin {
    public:
      using ValType = typename AttributeValueType<k>::ValType;
      void setValue( ValType&& v) {
        assert(!isSet());
        val = std::move(v);
        is_set = true;
      }

      void setValue(const ValType& v) {
        assert(!isSet());
        val = v;
        is_set = true;
      }
      
      auto& getValue() const { return val; }
      auto& getValue() { return val; }

      bool isSet() const {
        return is_set;
      }

      void unSet() {
        is_set = false;
      }

    private:
      ValType val;
      bool is_set = false;
    };

    template<class T>
    struct Keeper {
      Keeper () : isSet (false) {}
      Keeper (T&& set ) : value(std::forward(set)), isSet(true) {}
      void operator= (T set) {value= std::move(set); isSet = true; };
      
      void unset () {isSet = false;}
      auto& get () {return value;}
      const auto& get () const {return value;}
      
      operator bool () const {return isSet;}
    private:
      T value;
      bool isSet;

    };

    template<class T>
    inline std::ostream& operator<< (std::ostream& os, const Keeper<T>& s) {
      if (s) {
	return os << s.get ();
      }
      return os;
    }
    
    class Edge : public std::enable_shared_from_this<Edge> {
    public:
      Edge(Location_ptr from, Location_ptr to) : from(from),
						 to(to)
						  { }

      Edge(const Edge&) = default;

      /*template <AttributeType k>
      void setAttribute(const typename AttributeValueType<k>::ValType&& inp) {
        static_cast<EdgeAttributesMixin<k>*>(this)->setValue(std::move(inp));
      }

      template <AttributeType k>
      void setAttribute(const typename AttributeValueType<k>::ValType& inp) {
        static_cast<EdgeAttributesMixin<k>*>(this)->setValue(inp);
      }
      
      template <AttributeType k>
      void delAttribute() {
        static_cast<EdgeAttributesMixin<k>*>(this)->unSet();
      }

      template <AttributeType k>
      auto& getAttribute() {
        return static_cast<EdgeAttributesMixin<k>*>(this)->getValue();
      }

      template <AttributeType k>
      auto& getAttribute() const {
        return static_cast<const EdgeAttributesMixin<k>*>(this)->getValue();
      }

      template <AttributeType k>
      auto hasAttribute() const {
        return static_cast<const EdgeAttributesMixin<k>*>(this)->isSet();
      }
      */

      auto& getGuard () {return guard;}
      auto& getInstructions () {return instructions;}

      auto& getGuard () const {return guard;}
      auto& getInstructions () const  {return instructions;}
      
      
      auto getFrom() const { return from.lock(); }
      auto getTo() const { return to.lock(); }

      /** 
	   *  Set the to Location of this Edge. Also remove the edge
	   *  itself from the current to.
	   *
	   * @param t New target of the edge 
	   */
      void setTo(Location_ptr t) {
        to.lock()->removeIncomingEdge(this);
        to = t;
        t->addIncomingEdge(this);
      }

      //void setProgram (const Program_ptr& p) {prgm = p;}

      void copyAttributesFrom(const Edge& e) {
        guard = e.guard;
	instructions = e.instructions;
      }

    private:
      Location_wptr from;
      Location_wptr to;
      Value_ptr value;
      Keeper<Guard> guard;
      Keeper<InstructionStream> instructions;
    };

    inline std::ostream& operator<<(std::ostream& os, const Edge& e) {
      os << e.getGuard ();
      os << e.getInstructions ();
      return os;
    }

  } // namespace Model
} // namespace MiniMC

#endif
