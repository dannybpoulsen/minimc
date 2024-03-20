#ifndef _pathSTATE__
#define _pathSTATE__

#include "cpa/common.hpp"
#include "minimc/cpa/interface.hpp"
#include "smt/context.hpp"
#include "smt/solver.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/support/overload.hpp"

#include "pathvm/pathformua.hpp"
#include <cstring>
#include <memory>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      
      class QExpr : public MiniMC::CPA::QueryExpr {
      public:
        QExpr(MiniMC::VMT::Pathformula::Value&& val) : value(std::move(val)) {}
        std::ostream& output(std::ostream& os) const override {
          return os << value;
        }

        auto getValue() const { return value; }

      private:
        MiniMC::VMT::Pathformula::Value value;
      };
      
      
      class State : public MiniMC::CPA::DataState,
                    private MiniMC::CPA::QueryBuilder,
		    private MiniMC::CPA::LocationInfo
      {
      public:
        State(MiniMC::VMT::Pathformula::ActivationStack&& vals, MiniMC::VMT::Pathformula::Memory&& memory, SMTLib::Term_ptr&& formula, SMTLib::Context& ctxt) : call_stack(std::move(vals)),
                                                                                                                                      memory(std::move(memory)),
                                                                                                                                      pathformula(std::move(formula)),
                                                                                                                                      context(ctxt) {}
        State(const State& oth) : call_stack(oth.call_stack), memory(oth.memory), pathformula(oth.pathformula), context(oth.context), _hash(0) {}
        MiniMC::Hash::hash_t hash() const override {
	  static MiniMC::Hash::hash_t i= 0;
	  return ++i;
	}
	
        virtual std::shared_ptr<MiniMC::CPA::DataState> copy() const override { return std::make_shared<State>(*this); }

        const Solver_ptr getConcretizer() const override;
        auto& getStack() { return call_stack; }
        auto& getMemory() { return memory; }
        auto& getStack() const { return call_stack; }

	MiniMC::Model::Location& getLocation(proc_id) const override   {return *getStack().back().getLocation();}
	size_t nbOfProcesses() const override {return 1;}
	bool isActive(size_t) const override {return getStack().getDepth();}
	const MiniMC::CPA::LocationInfo& getLocationState () const {return *this;}
	
        void addConstraints(const SMTLib::Term_ptr& term) {
          pathformula = context.getBuilder().buildTerm(SMTLib::Ops::And, {pathformula, term});
        }

        auto& getPathformula() const { return pathformula; }

        virtual const QueryBuilder& getBuilder() const { return *this; }
        virtual QueryExpr_ptr buildValue(MiniMC::Model::proc_t p, const MiniMC::Model::Value_ptr& val) const override {
          if (p > 0) {
            throw MiniMC::Support::Exception("Not enough processes");
          }
	  MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::Value> metas{1};  
	  MiniMC::VMT::Pathformula::ValueLookup lookup {MiniMC::VMT::Pathformula::ValueCreator{context.getBuilder ()},{const_cast<MiniMC::VMT::Pathformula::ActivationStack&> (call_stack),metas}};
          return std::make_unique<QExpr>(lookup.lookupValue(*val));
        }

      private:
	MiniMC::VMT::Pathformula::ActivationStack call_stack;
        MiniMC::VMT::Pathformula::Memory memory;
        SMTLib::Term_ptr pathformula;
        SMTLib::Context& context;
        mutable MiniMC::Hash::hash_t _hash = 0;
      };

      class Concretizer : public MiniMC::CPA::Solver {
      public:
        Concretizer(const State& s, SMTLib::Solver_ptr&& solver) : state(s), solver(std::move(solver)) {
	  this->solver->assert_formula(state.getPathformula());

	}
	
        Feasibility isFeasible() const override {
          switch (solver->check_sat()) {
            case SMTLib::Result::Satis:
              return Feasibility::Feasible;
            case SMTLib::Result::NSatis:
              return Feasibility::Infeasible;
            default:
              return Feasibility::Unknown;
          }
        }

	MiniMC::Model::Constant_ptr evaluate(const QueryExpr& expr) const override {
          auto& myexpr = static_cast<const QExpr&>(expr);
          if (isFeasible() == Feasibility::Feasible) {
	    return MiniMC::VMT::Pathformula::Value::visit (MiniMC::Support::Overload {
		[this](MiniMC::VMT::Pathformula::Value::I8& val) ->MiniMC::Model::Constant_ptr {
		  return std::make_shared<MiniMC::Model::I8Integer> (val.interpretValue (*solver));
		},
		  [this](MiniMC::VMT::Pathformula::Value::I16& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I16Integer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::Value::I32& val) ->MiniMC::Model::Constant_ptr { return std::make_shared<MiniMC::Model::I32Integer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::Value::I64& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::I64Integer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::Value::Pointer& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::Pointer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::Value::Pointer32& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::Pointer32> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::Value::Bool& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::Bool> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::Value::Aggregate& val) ->MiniMC::Model::Constant_ptr {
		    auto res = val.interpretValue (*solver);
		    return std::make_shared<MiniMC::Model::AggregateConstant> (std::move(res));;
		  }
		  },
	      myexpr.getValue ()
	      );
	      
          }
	  
	  throw MiniMC::Support::VerificationException {"Cannot evalute values on unsatisfiable states"};
        }

      private:
        const State& state;
        SMTLib::Solver_ptr solver;
      };

      const Solver_ptr State::getConcretizer() const {
        auto solver = context.makeSolver();
	solver->assert_formula(getPathformula());
        return std::make_shared<Concretizer>(*this, std::move(solver));
      }

    } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC

#endif
