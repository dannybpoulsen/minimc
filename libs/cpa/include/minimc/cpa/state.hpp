/**
 * @file   state.hpp
 * @date   Mon Apr 20 17:21:09 2020
 * 
 * @brief  
 * 
 * 
 */

#ifndef _STATE__
#define _STATE__

#include "minimc/hash/hashing.hpp"
#include "minimc/model/location.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/support/localisation.hpp"
#include "minimc/model/array.hpp"
#include "minimc/cpa/query.hpp"
#include <memory>
#include <ranges>
#include "minimc/io/ostream.hpp"

namespace MiniMC {
  namespace CPA {

    using proc_id = std::size_t;
    
    class Solver : public QueryEvaluator {
    public:
      enum class Feasibility {
        Feasible,
        Infeasible,
        Unknown
      };

      virtual ~Solver () {}
      
      virtual Feasibility isFeasible() const  {return Feasibility::Unknown;}
      virtual MiniMC::Model::Constant_ptr evaluate (const QueryExpr&) const override = 0;
      
    };
    
    using Solver_ptr = std::shared_ptr<Solver>;
    
    struct LocationInfo {
      virtual MiniMC::Model::Location& getLocation(proc_id) const  = 0;
      virtual size_t nbOfProcesses() const = 0;
      virtual bool isActive(size_t i) const = 0;
    
    };

    class State;
    using State_ptr = std::shared_ptr<State>;

    template<class S,typename... Args>
    std::shared_ptr<S> makeState (Args&&... args) {return std::make_shared<S> (std::forward<Args> (args)...);};
    
    class State 
    {
    public:
      virtual ~State () {}
      virtual const Solver_ptr getConcretizer() const = 0;
      virtual const QueryBuilder& getBuilder () const = 0;
      virtual const LocationInfo& getLocationState () const  = 0;
      virtual State_ptr copy() const = 0;
      virtual MiniMC::Hash::hash_t hash() const = 0;
    };
    
    
    
    
    class CPAStateOutputter {
    public:
      CPAStateOutputter (const MiniMC::Model::Program& prgm) : prgm(prgm) {}
      MiniMC::IO::ostream& output (const State&, MiniMC::IO::ostream& os);
    private:
      const MiniMC::Model::Program& prgm;
      
    };
    
    
    
    
  } // namespace CPA
} // namespace MiniMC

#endif
