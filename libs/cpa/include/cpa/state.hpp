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

#include "hash/hashing.hpp"
#include "model/location.hpp"
#include "model/variables.hpp"
#include "model/cfg.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "util/array.hpp"
#include "cpa/query.hpp"
#include <iosfwd>
#include <memory>
#include <ranges>

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
      
    };
    

    
    
    
    class CFAState : public std::enable_shared_from_this<CFAState> {
    public:
      virtual ~CFAState () {}
      virtual const LocationInfo& getLocationState () const  = 0;
      virtual std::shared_ptr<CFAState> copy() const = 0;
      virtual MiniMC::Hash::hash_t hash() const = 0;
    
    };
    
    
    class DataState 
    {
    public:
      virtual ~DataState () {}
      virtual const Solver_ptr getConcretizer() const = 0;
      virtual const QueryBuilder& getBuilder () const = 0;
      virtual std::shared_ptr<DataState> copy() const = 0;
      virtual MiniMC::Hash::hash_t hash() const = 0;
      
    };

    template<class T>
    using State_ptr = std::shared_ptr<const T>;
    
    using DataState_ptr = State_ptr<DataState>;
    using CFAState_ptr = std::shared_ptr<const CFAState>;
    
    
    class AnalysisState  {
    public:
      AnalysisState () {}
      AnalysisState (CFAState_ptr&& cfa, std::vector<DataState_ptr>&& datastates) : cfastate(std::move(cfa)),datastates(std::move(datastates)) {}
      auto& getCFAState () const {return *cfastate;}
      auto dataStates () const {
	return datastates | std::views::transform([](auto& r)->const DataState& {return *r;});;
      }
      MiniMC::Hash::hash_t hash() const;
    private:
      CFAState_ptr cfastate;
      std::vector<DataState_ptr> datastates;   
    };

    class StateOutputter {
    public:
      StateOutputter (const MiniMC::Model::Program& prgm) : prgm(prgm) {}
      std::ostream& output (const AnalysisState&, std::ostream& os);
    private:
      const MiniMC::Model::Program& prgm;
      
    };
    
    std::ostream& operator<<(std::ostream& os, const AnalysisState& state);
    
    
    
  } // namespace CPA
} // namespace MiniMC

#endif
