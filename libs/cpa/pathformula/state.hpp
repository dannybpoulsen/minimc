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
        QExpr(MiniMC::VMT::Pathformula::PathFormulaVMVal&& val) : value(std::move(val)) {}
        std::ostream& output(std::ostream& os) const override {
          return os << value;
        }

        auto getValue() const { return value; }

      private:
        MiniMC::VMT::Pathformula::PathFormulaVMVal value;
      };

      class StackControl : public MiniMC::VMT::StackControl<MiniMC::VMT::Pathformula::PathFormulaVMVal> {
      public:
        StackControl(MiniMC::VMT::Pathformula::ActivationStack& stack, SMTLib::Context& context) : stack(stack), context(context) {}
        // StackControl API
        void push(MiniMC::Model::Location_ptr, std::size_t registers, const MiniMC::Model::Value_ptr& ret) override {
          MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::PathFormulaVMVal> values{registers};
	  stack.push({std::move(values), ret});
        }

        void pop(MiniMC::VMT::Pathformula::PathFormulaVMVal&& val) override {
          auto ret = stack.back().ret;
          stack.pop();
	  if (ret) 
	    stack.back().values.set (*std::static_pointer_cast<MiniMC::Model::Register>(ret), std::move(val));
        }

        void popNoReturn() override {
          stack.pop();
        }

      private:
        MiniMC::VMT::Pathformula::ActivationStack& stack;
        SMTLib::Context& context;
      };

      class State : public MiniMC::CPA::DataState,
                    private MiniMC::CPA::QueryBuilder {
      public:
        State(MiniMC::VMT::Pathformula::ActivationStack&& vals, MiniMC::VMT::Pathformula::Memory&& memory, SMTLib::Term_ptr&& formula, SMTLib::Context& ctxt) : call_stack(std::move(vals)),
                                                                                                                                      memory(std::move(memory)),
                                                                                                                                      pathformula(std::move(formula)),
                                                                                                                                      context(ctxt) {}
        State(const State& oth) : call_stack(oth.call_stack), memory(oth.memory), pathformula(oth.pathformula), context(oth.context), _hash(0) {}
        MiniMC::Hash::hash_t hash() const override {
          // Hashes for pathformula states makes no sense
          // Since they should be usable by all algorithms we just makes a fake hash that increments with each invocation of the hash function
          static MiniMC::Hash::hash_t nextHash{0};
          if (!_hash)
            _hash = ++nextHash;
          return _hash;
        }
	
        virtual std::shared_ptr<MiniMC::CPA::DataState> copy() const override { return std::make_shared<State>(*this); }

        const Solver_ptr getConcretizer() const override;
        auto& getStack() { return call_stack; }
        auto& getMemory() { return memory; }
        auto& getStack() const { return call_stack; }

        void addConstraints(const SMTLib::Term_ptr& term) {
          pathformula = context.getBuilder().buildTerm(SMTLib::Ops::And, {pathformula, term});
        }

        auto& getPathformula() const { return pathformula; }

        virtual const QueryBuilder& getBuilder() const { return *this; }
        virtual QueryExpr_ptr buildValue(MiniMC::Model::proc_t p, const MiniMC::Model::Value_ptr& val) const override {
          if (p > 0) {
            throw MiniMC::Support::Exception("Not enough processes");
          }
	  MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::PathFormulaVMVal> metas{1};  
	  MiniMC::VMT::Pathformula::ValueLookup lookup {const_cast<MiniMC::VMT::Pathformula::ActivationStack&> (call_stack),metas,context.getBuilder ()};
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
	    return MiniMC::VMT::Pathformula::PathFormulaVMVal::visit (MiniMC::Support::Overload {
		[this](MiniMC::VMT::Pathformula::PathFormulaVMVal::I8& val) ->MiniMC::Model::Constant_ptr {
		  return std::make_shared<MiniMC::Model::I8Integer> (val.interpretValue (*solver));
		},
		  [this](MiniMC::VMT::Pathformula::PathFormulaVMVal::I16& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I16Integer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::PathFormulaVMVal::I32& val) ->MiniMC::Model::Constant_ptr { return std::make_shared<MiniMC::Model::I32Integer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::PathFormulaVMVal::I64& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::I64Integer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::Pointer> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer32& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::Pointer32> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::PathFormulaVMVal::Bool& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::Bool> (val.interpretValue (*solver));},
		  [this](MiniMC::VMT::Pathformula::PathFormulaVMVal::Aggregate& val) ->MiniMC::Model::Constant_ptr {
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
