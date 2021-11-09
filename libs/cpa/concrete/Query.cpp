#include <gsl/pointers>
#include <memory>

#include "cpa/concrete.hpp"
#include "hash/hashing.hpp"
#include "heap.hpp"
#include "instructionimpl.hpp"
#include "util/vm.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      class MConcretizer : public MiniMC::CPA::Concretizer {
      public:
        MConcretizer(const std::vector<VariableLookup>& v) :  vars(v) {}
        virtual MiniMC::CPA::Concretizer::Feasibility isFeasible() const override { return Feasibility::Feasible; }

        virtual std::ostream& evaluate_str(proc_id id, const MiniMC::Model::Variable_ptr& var, std::ostream& os) {
	  return os << vars.at(id).at(var);          
        }

        virtual MiniMC::Util::Array evaluate(proc_id id, const MiniMC::Model::Variable_ptr& var) override {
	  return vars.at(id).at(var);
        }

      private:
        const std::vector<VariableLookup>& vars;
      };

      class State : public MiniMC::CPA::State, public MiniMC::CPA::Concretizer {
      public:
        State( const std::vector<VariableLookup>& var) :  proc_vars(var) {
        }
        virtual std::ostream& output(std::ostream& os) const {
          for (auto& vl : proc_vars) {
            os << "===\n";
            os << vl << "\n";
          }
          return os << "==\n";
        }

        virtual MiniMC::Hash::hash_t hash(MiniMC::Hash::seed_t seed = 0) const override {
          if (!hash_val) {
            for (auto& vl : proc_vars) {
              MiniMC::Hash::hash_combine(seed, vl);
            }
            MiniMC::Hash::hash_combine(seed, heap);
            //uncommnented the update of this buffered hash value. It
            //disables the buffering as it might be incorrect
            //The State is really just a container and the parts
            //making up its hash-values can actually change outside
            //its knowledge....making it impossible to keep
            //precomputed hash_value up to date
            //hash_val = seed;
            return seed;
          }
          return hash_val;
        }

        virtual std::shared_ptr<MiniMC::CPA::State> copy() const override {
          return std::make_shared<State>(*this);
        }

        auto& getProc(std::size_t i) { return proc_vars[i]; }
        auto& getHeap() { return heap; }

        auto& getProc(std::size_t i) const { return proc_vars[i]; }
        auto& getHeap() const { return heap; }

        virtual bool need2Store() const override { return false; }
        virtual bool ready2explore() const override  { return true; }
        virtual bool assertViolated() const override  { return false; }

        virtual const Concretizer_ptr getConcretizer() const override { return std::make_shared<MConcretizer>(proc_vars); }

      private:
        std::vector<VariableLookup> proc_vars;
        Heap heap;
        mutable MiniMC::Hash::hash_t hash_val = 0;
      };

      MiniMC::CPA::State_ptr StateQuery::makeInitialState(const MiniMC::Model::Program& p) {
        
        std::vector<VariableLookup> stack;
        for (auto& f : p.getEntryPoints()) {
          auto& vstack = f->getVariableStackDescr();
          stack.emplace_back(vstack->getTotalVariables());
          for (auto& v : vstack->getVariables()) {
            MiniMC::Util::Array arr(v->getType()->getSize());
            stack.back()[v] = arr;
            assert(stack.back()[v].getSize() == v->getType()->getSize());
          }
        }

        auto state = std::make_shared<State>(stack);

	for (auto block : p.getHeapLayout ()) {
	  state->getHeap ().allocate (block.size);
	}
	
        VMData data{
            .readFrom = {.local = nullptr, .heap = &state->getHeap()},
            .writeTo = { .local = nullptr, .heap = &state->getHeap()}};

        auto it = p.getInitialisation().begin();
        auto end = p.getInitialisation().end();
        MiniMC::Util::runVM<decltype(it), VMData, ExecuteInstruction>(it, end, data);

        return state;
      }

      MiniMC::CPA::State_ptr Transferer::doTransfer(const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id) {
        auto resstate = s->copy();
        auto& ostate = static_cast<const MiniMC::CPA::Concrete::State&>(*s);
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);

        VMData data{
            .readFrom = {
                .local = const_cast<VariableLookup*>(&nstate.getProc(id)),
                .heap = &nstate.getHeap()},
            .writeTo = { .local = &nstate.getProc(id), .heap = &nstate.getHeap()}};

        if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {

          auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions>();
          try {

            if (instr.isPhi) {
              data.readFrom.local = const_cast<VariableLookup*>(&ostate.getProc(id));
              data.readFrom.heap = const_cast<Heap*>(&ostate.getHeap());
            }
            auto it = instr.begin();
            auto end = instr.end();

            MiniMC::Util::runVM<decltype(it), VMData, ExecuteInstruction>(it, end, data);

          } catch (MiniMC::Support::AssumeViolated&) {
            return nullptr;
          }
        }
        return resstate;
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
