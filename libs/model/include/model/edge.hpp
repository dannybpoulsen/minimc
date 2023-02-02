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

    class Instruction;
    
    class Edge : public std::enable_shared_from_this<Edge> {
    public:
      Edge(Location_ptr from, Location_ptr to, InstructionStream&& stream = {},bool isPhi = false) : from(from),
												     to(to),
												     instructions(std::move(stream)),
												     phi(isPhi)
									   
      { }

      Edge(const Edge&) = default;
      
      auto& getInstructions () {return instructions;}

      auto& getInstructions () const  {return instructions;}
      
      
      auto getFrom() const { return from; }
      auto getTo() const { return to; }

      auto isPhi () const {return phi;}
      
    private:
      Location_ptr from;
      Location_ptr to;
      Value_ptr value;
      InstructionStream instructions;
      bool phi = false;
    };

    inline std::ostream& operator<<(std::ostream& os, const Edge& e) {
      os << e.getInstructions ();
      return os;
    }

  } // namespace Model
} // namespace MiniMC

#endif
