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
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "util/array.hpp"
#include "cpa/query.hpp"
#include <iosfwd>
#include <memory>

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
    
    class CommonState   {
    public:
      ~CommonState() {}
      
      virtual std::ostream& output(std::ostream& os) const { return os << "_"; }
      virtual MiniMC::Hash::hash_t hash() const = 0;
    };


    
    
    class CFAState : public CommonState,
		     public std::enable_shared_from_this<CFAState> {
    public:
      virtual ~CFAState () {}
      virtual const LocationInfo& getLocationState () const  = 0;
      virtual std::shared_ptr<CFAState> copy() const = 0;
      
    };
    
    
    class DataState : public CommonState
    {
    public:
      virtual ~DataState () {}
      virtual const Solver_ptr getConcretizer() const = 0;
      virtual const QueryBuilder& getBuilder () const = 0;
      virtual std::shared_ptr<DataState> copy() const = 0;
    };

    template<class T>
    using State_ptr = std::shared_ptr<const T>;
    
    using CommonState_ptr = State_ptr<CommonState>;
    using DataState_ptr = State_ptr<DataState>;
    using CFAState_ptr = std::shared_ptr<const CFAState>;
    
    
    class AnalysisState  {
    public:
      AnalysisState () {}
      AnalysisState (CFAState_ptr&& cfa, std::vector<DataState_ptr>&& datastates) : cfastate(std::move(cfa)),datastates(std::move(datastates)) {}
      auto& getCFAState () const {return cfastate;}
      auto& getDataState (std::size_t i) const {return datastates.at(i);}
      std::size_t nbDataStates () const {return datastates.size ();}
      MiniMC::Hash::hash_t hash() const;
    private:
      CFAState_ptr cfastate;
      std::vector<DataState_ptr> datastates;   
    };
    
    std::ostream& operator<<(std::ostream& os, const CommonState& state);
    std::ostream& operator<<(std::ostream& os, const AnalysisState& state);
    
    
    
  } // namespace CPA
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::CPA::CommonState> {
    std::size_t operator()(const MiniMC::CPA::CommonState& s) const noexcept {
      return s.hash();
    }
  };

} // namespace std

#endif
