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
#include <iosfwd>
#include <memory>

namespace MiniMC {
  namespace CPA {

    using proc_id = std::size_t;


    /* Values returned by states when looking up Variable Values.
       CFA-implmenentations should implement this structure
    */
    class ByteVectorExpr {
    public:
      ByteVectorExpr (std::size_t s) : size(s) {}
      virtual ~ByteVectorExpr () {}
      virtual std::ostream& output (std::ostream& os) const {return os << "??";}
      std::size_t getSize () const {return size;}
    private:
      std::size_t size;
    };

    inline std::ostream& operator<< (std::ostream& os, const ByteVectorExpr&  expr) {
      return expr.output (os);
    }
    
    template<class T>
    class TByteVectorExpr :  public ByteVectorExpr {
    public:
      TByteVectorExpr (T&& t, std::size_t s) : ByteVectorExpr(s),val(std::move(t)) {}
      TByteVectorExpr (const T& t, std::size_t s) : ByteVectorExpr(s),val(t) {}
      
      std::ostream& output (std::ostream& os ) const override {return os << val;}
    private:
      std::size_t size;
      T val;
    };
    
    using ByteVectorExpr_ptr  = std::unique_ptr<ByteVectorExpr>;
    
    class CanntEvaluateException : public MiniMC::Support::VerificationException {
    public:
      CanntEvaluateException(const MiniMC::Model::Register_ptr& var) : VerificationException(MiniMC::Support::Localiser("Cannot Evaluate '%1%' to a value").format(var->getName())) {}
    };
    
    class Solver {
    public:
      enum class Feasibility {
        Feasible,
        Infeasible,
        Unknown
      };

      virtual ~Solver () {}
      
      virtual Feasibility isFeasible() const  {return Feasibility::Unknown;}
    };
    
    using Solver_ptr = std::shared_ptr<Solver>;

    struct LocationInfo {
      template <MiniMC::Model::Attributes att>
      bool hasLocationOf() const { return hasLocationAttribute(static_cast<MiniMC::Model::AttrType>(att)); }

      virtual bool need2Store() const { return hasLocationAttribute(static_cast<MiniMC::Model::AttrType>(MiniMC::Model::Attributes::NeededStore)); }
      virtual bool assertViolated() const { return hasLocationAttribute(static_cast<MiniMC::Model::AttrType>(MiniMC::Model::Attributes::AssertViolated)); }
      virtual bool hasLocationAttribute(MiniMC::Model::AttrType) const { return false; }
      
      virtual MiniMC::Model::Location_ptr getLocation(proc_id) const  = 0;
      virtual size_t nbOfProcesses() const = 0;
      
    };
    
    /** A general CPA state interface. It is deliberately kept minimal to relay no information to observers besides what is absolutely needed 
     * 
     */
    class State : public std::enable_shared_from_this<State> {
    public:
      ~State() {}

      virtual std::ostream& output(std::ostream& os) const { return os << "_"; }
      virtual MiniMC::Hash::hash_t hash() const = 0;
      virtual std::shared_ptr<State> copy() const = 0;

      
      
      virtual ByteVectorExpr_ptr symbEvaluate (proc_id, const MiniMC::Model::Register_ptr& v) const  {
	return std::make_unique<ByteVectorExpr> ( v->getType ()->getSize ());
      }
      
      virtual const LocationInfo& getLocationState () const {
	throw MiniMC::Support::Exception ("LocationState should not be called");
      } 
      virtual const Solver_ptr getConcretizer() const {return std::make_shared<Solver> ();}
    };

    using State_ptr = std::shared_ptr<State>;

    std::ostream& operator<<(std::ostream& os, const State& state);

  } // namespace CPA
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::CPA::State> {
    std::size_t operator()(const MiniMC::CPA::State& s) const noexcept {
      return s.hash();
    }
  };

} // namespace std

#endif
