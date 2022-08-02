#ifndef _pathSTATE__
#define _pathSTATE__

#include "cpa/common.hpp"
#include "cpa/interface.hpp"
#include "smt/context.hpp"
#include "smt/solver.hpp"
#include "support/feedback.hpp"
#include "support/pointer.hpp"
#include "vm/pathformula/pathformua.hpp"
#include <cstring>
#include <memory>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      template <class T>
      struct MapToConcrete;

      template <>
      struct MapToConcrete<MiniMC::VMT::Pathformula::PathFormulaVMVal::I64> {
        using Concrete = MiniMC::VMT::Concrete::ConcreteVMVal::I64;
      };

      template <>
      struct MapToConcrete<MiniMC::VMT::Pathformula::PathFormulaVMVal::I32> {
        using Concrete = MiniMC::VMT::Concrete::ConcreteVMVal::I32;
      };

      template <>
      struct MapToConcrete<MiniMC::VMT::Pathformula::PathFormulaVMVal::I16> {
        using Concrete = MiniMC::VMT::Concrete::ConcreteVMVal::I16;
      };

      template <>
      struct MapToConcrete<MiniMC::VMT::Pathformula::PathFormulaVMVal::I8> {
        using Concrete = MiniMC::VMT::Concrete::ConcreteVMVal::I8;
      };

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

      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::PathFormulaVMVal, MiniMC::VMT::Pathformula::ValueLookup>;

      using ActivationStack = MiniMC::CPA::Common::ActivationStack<ActivationRecord>;

      class StackControl : public MiniMC::VMT::StackControl<MiniMC::VMT::Pathformula::PathFormulaVMVal> {
      public:
        StackControl(ActivationStack& stack, SMTLib::Context& context) : stack(stack), context(context) {}
        // StackControl API
        void push(std::size_t registers, const MiniMC::Model::Value_ptr& ret) override {
          MiniMC::VMT::Pathformula::ValueLookup values{registers, context.getBuilder()};
          stack.push({std::move(values), ret});
        }

        void pop(MiniMC::VMT::Pathformula::PathFormulaVMVal&& val) override {
          auto ret = stack.back().ret;
          stack.pop();
          stack.back().values.saveValue(*std::static_pointer_cast<MiniMC::Model::Register>(ret), std::move(val));
        }

        void popNoReturn() override {
          stack.pop();
        }

        MiniMC::VMT::ValueLookup<MiniMC::VMT::Pathformula::PathFormulaVMVal>& getValueLookup() override { return stack.back().values; }

      private:
        ActivationStack& stack;
        SMTLib::Context& context;
      };

      class State : public MiniMC::CPA::DataState,
                    private MiniMC::CPA::QueryBuilder {
      public:
        State(ActivationStack&& vals, MiniMC::VMT::Pathformula::Memory&& memory, SMTLib::Term_ptr&& formula, SMTLib::Context& ctxt) : call_stack(std::move(vals)),
                                                                                                                                      memory(std::move(memory)),
                                                                                                                                      pathformula(std::move(formula)),
                                                                                                                                      context(ctxt) {}
        State(const State& oth) : call_stack(oth.call_stack), memory(oth.memory), pathformula(oth.pathformula), context(oth.context), _hash(0) {}
        virtual std::ostream& output(std::ostream& os) const override { return os << "\nPathformula:" << *pathformula; }
        MiniMC::Hash::hash_t hash() const override {
          // Hashes for pathformula states makes no sense
          // Since they should be usable by all algorithms we just makes a fake hash that increments with each invocation of the hash function
          static MiniMC::Hash::hash_t nextHash{0};
          if (!_hash)
            _hash = ++nextHash;
          return _hash;
        }
        virtual std::shared_ptr<MiniMC::CPA::CommonState> copy() const override { return std::make_shared<State>(*this); }

        const Solver_ptr getConcretizer() const override;
        auto& getStack() { return call_stack; }
        auto& getMemory() { return memory; }
        auto& getStack() const { return call_stack; }

        void addConstraints(SMTLib::Term_ptr& term) {
          pathformula = context.getBuilder().buildTerm(SMTLib::Ops::And, {pathformula, term});
        }

        auto& getPathformula() const { return pathformula; }

        virtual const QueryBuilder& getBuilder() const { return *this; }
        virtual QueryExpr_ptr buildValue(MiniMC::proc_t p, const MiniMC::Model::Value_ptr& val) const override {
          if (p > 0) {
            throw MiniMC::Support::Exception("Not enough processes");
          }
          return std::make_unique<QExpr>(call_stack.back().values.lookupValue(val));
        }

      private:
        ActivationStack call_stack;
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

        MiniMC::VMT::Concrete::ConcreteVMVal evaluate(const QueryExpr& expr) const override {
          auto& myexpr = static_cast<const QExpr&>(expr);
          if (isFeasible() == Feasibility::Feasible) {
            return myexpr.getValue().visit([this](auto& t) -> MiniMC::VMT::Concrete::ConcreteVMVal {
              auto& term = t.getTerm();
              if constexpr (std::is_same_v<decltype(t), MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer&>) {
                MiniMC::pointer_t pointer;
                // std::memset (&pointer,0,sizeof(MiniMC::pointer_t));
		
                auto pointerres = std::get<SMTLib::bitvector>(solver->getModelValue(term));
                assert(sizeof(MiniMC::pointer_t) == pointerres.size() / 8);
		auto beginoff = pointerres.begin()+((sizeof(MiniMC::pointer_t)-offsetof(MiniMC::pointer_t,offset)-sizeof(pointer.offset)))*8;
		auto segoff = pointerres.begin()+((sizeof(MiniMC::pointer_t)-offsetof(MiniMC::pointer_t,segment)-sizeof(pointer.segment)))*8;
		auto baseoff = pointerres.begin()+((sizeof(MiniMC::pointer_t)-offsetof(MiniMC::pointer_t,base)-sizeof(pointer.base)))*8;
		auto zerooff = pointerres.begin()+((sizeof(MiniMC::pointer_t)-offsetof(MiniMC::pointer_t,zero)-sizeof(pointer.zero)))*8;
		
		
		MiniMC::Support::SMT::extractBytes(beginoff, beginoff+sizeof(pointer.offset)*8, reinterpret_cast<MiniMC::BV8*>(&pointer.offset));
		MiniMC::Support::SMT::extractBytes(segoff, segoff+sizeof(pointer.segment)*8, reinterpret_cast<MiniMC::BV8*>(&pointer.segment));
                MiniMC::Support::SMT::extractBytes(baseoff, baseoff+sizeof(pointer.base)*8, reinterpret_cast<MiniMC::BV8*>(&pointer.base));
                MiniMC::Support::SMT::extractBytes(zerooff, zerooff+sizeof(pointer.zero)*8, reinterpret_cast<MiniMC::BV8*>(&pointer.zero));
                
		return MiniMC::VMT::Concrete::ConcreteVMVal::Pointer{pointer};
              }

              if constexpr (std::is_same_v<decltype(t), MiniMC::VMT::Pathformula::PathFormulaVMVal::Aggregate&>) {
                MiniMC::Util::Array res{t.size()};

                auto aggrres = std::get<SMTLib::bitvector>(solver->getModelValue(term));
                MiniMC::Support::SMT::extractBytes(aggrres.begin(), aggrres.end(), res.get_direct_access());
                return MiniMC::VMT::Concrete::ConcreteVMVal::Aggregate{std::move(res)};
              }

              if constexpr (std::is_same_v<decltype(t), MiniMC::VMT::Pathformula::PathFormulaVMVal::I64&> ||
                            std::is_same_v<decltype(t), MiniMC::VMT::Pathformula::PathFormulaVMVal::I32&> ||
                            std::is_same_v<decltype(t), MiniMC::VMT::Pathformula::PathFormulaVMVal::I16&> ||
                            std::is_same_v<decltype(t), MiniMC::VMT::Pathformula::PathFormulaVMVal::I8&>)

              {
                using ConcreteVMType = MapToConcrete<std::remove_reference_t<decltype(t)>>::Concrete;
                typename ConcreteVMType::underlying_type res{0};
                // std::memset (&pointer,0,sizeof(MiniMC::pointer_t));

                auto ires = std::get<SMTLib::bitvector>(solver->getModelValue(term));
                assert(sizeof(typename ConcreteVMType::underlying_type) == ires.size() / 8);
                MiniMC::Support::SMT::extractBytes(ires.begin(), ires.end(), reinterpret_cast<MiniMC::BV8*>(&res));
                return ConcreteVMType{res};
              }

              if constexpr (std::is_same_v<decltype(t), MiniMC::VMT::Pathformula::PathFormulaVMVal::Bool&>)

		{
		  MiniMC::Support::Messager mess{};
		  mess.message<MiniMC::Support::Severity::Error> ("Evaluation for Boolean values not properly implemented. Don't trust this value");
                return MiniMC::VMT::Concrete::ConcreteVMVal::I64(100);
              }

              throw MiniMC::Support::Exception("Shouldn't get here ");
            });
          }
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
