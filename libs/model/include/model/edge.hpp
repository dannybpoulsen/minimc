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

    class Edge : public std::enable_shared_from_this<Edge> {
    public:
      Edge(Location_ptr from, Location_ptr to) : from(from),
						 to(to)
						  { }

      Edge(const Edge&) = default;

      //auto& getGuard () {return guard;}
      auto& getInstructions () {return instructions;}

      //auto& getGuard () const {return guard;}
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
	instructions = e.instructions;
      }

    private:
      Location_wptr from;
      Location_wptr to;
      Value_ptr value;
      InstructionStream instructions;
    };

    inline std::ostream& operator<<(std::ostream& os, const Edge& e) {
      //os << e.getGuard ();
      os << e.getInstructions ();
      return os;
    }

  } // namespace Model
} // namespace MiniMC

#endif
