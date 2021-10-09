
#include <set>
#include <sstream>
#include <vector>

#include "cpa/interface.hpp"
#include "cpa/state.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/graph.hpp"
#include "support/queue_stack.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ARG {
      class State : public MiniMC::CPA::State {
      public:
        struct Parent {
          std::weak_ptr<State> from;
          proc_id who;
          MiniMC::Model::Edge_ptr edge;
        };
        State(MiniMC::CPA::State_ptr wrapped) : wrappedState(wrapped) {}
        State(const State& s) : wrappedState(s.wrappedState), parents(s.parents) {}

        virtual std::ostream& output(std::ostream& os) const  override { return wrappedState->output(os); }
        virtual MiniMC::Hash::hash_t hash(MiniMC::Hash::seed_t seed = 0) const override { return wrappedState->hash(seed); }
        virtual std::shared_ptr<MiniMC::CPA::State> copy() const override  { return std::make_shared<State>(*this); }
        virtual bool need2Store() const override { return wrappedState->need2Store(); }
        virtual bool ready2explore() const override { return wrappedState->ready2explore(); }
        virtual bool assertViolated() const override { return wrappedState->assertViolated(); }
        virtual MiniMC::Model::Location_ptr getLocation(proc_id id) const override {
          return wrappedState->getLocation(id);
        }

        virtual bool hasLocationAttribute(MiniMC::Model::AttrType tt) const override {
          return wrappedState->hasLocationAttribute(tt);
        }

        size_t nbOfProcesses() const override {
          return wrappedState->nbOfProcesses();
        }

        virtual const Concretizer_ptr getConcretizer() const override {
          return wrappedState->getConcretizer();
        }

        auto parent_inserter() { return std::back_inserter(parents); }
        auto begin() { return parents.begin(); }
        auto end() { return parents.end(); }
        auto& getWrapped() { return wrappedState; }

      private:
        MiniMC::CPA::State_ptr wrappedState;
        std::vector<Parent> parents;
      };

      struct StateQuery : public MiniMC::CPA::StateQuery {
        StateQuery(const MiniMC::CPA::StateQuery_ptr& g) : wrapped(g) {}
        State_ptr makeInitialState(const MiniMC::Model::Program& prgm) {
          return std::make_shared<State>(wrapped->makeInitialState(prgm));
        }

      private:
        MiniMC::CPA::StateQuery_ptr wrapped;
      };

      struct Transferer : public MiniMC::CPA::Transferer {
        Transferer(const MiniMC::CPA::Transferer_ptr& p) : wrappedt(p) {}
        State_ptr doTransfer(const State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id) {

          auto ns = std::static_pointer_cast<State>(s);
          auto wrapped = ns->getWrapped();
          auto wres = wrappedt->doTransfer(wrapped, e, id);
          if (wres) {
            auto res = std::make_shared<State>(wres);
            res->parent_inserter() = {.from = ns, .who = id, .edge = e};
            return res;
          }
          return nullptr;
        }

      private:
        MiniMC::CPA::Transferer_ptr wrappedt;
      };

      struct Joiner : public MiniMC::CPA::Joiner {
        Joiner(const MiniMC::CPA::Joiner_ptr& g) : wrapped(g) {}
        State_ptr doJoin(const State_ptr& l, const State_ptr& r) {
          auto nl = std::static_pointer_cast<State>(l);
          auto nr = std::static_pointer_cast<State>(r);

          auto wres = wrapped->doJoin(nl->getWrapped(), nr->getWrapped());
          if (wres) {
            auto res = std::make_shared<State>(wres);

            auto inserter = res->parent_inserter();
            auto insertFunction = [&](auto it) { inserter = it; };
            std::for_each(nl->begin(), nl->end(), insertFunction);
            std::for_each(nr->begin(), nr->end(), insertFunction);
            return res;
          }
          return nullptr;
        }

        bool covers(const State_ptr& l, const State_ptr& r) {
          auto nl = std::static_pointer_cast<State>(l);
          auto nr = std::static_pointer_cast<State>(r);
          if (wrapped->covers(nl->getWrapped(), nr->getWrapped())) {
            coverCopy(std::static_pointer_cast<MiniMC::CPA::State>(nr), std::static_pointer_cast<MiniMC::CPA::State>(l));
            return true;
          }
          return false;
        }

        void coverCopy(const State_ptr& from, const State_ptr& to) {
          auto nfrom = std::static_pointer_cast<State>(from);
          auto nto = std::static_pointer_cast<State>(to);
          auto inserter = nto->parent_inserter();
          auto insertFunction = [&](auto it) { inserter = it; };
          std::for_each(nfrom->begin(), nfrom->end(), insertFunction);
        }

      private:
        const MiniMC::CPA::Joiner_ptr wrapped;
      };

      template <class Iterator>
      void generateARGGraph(MiniMC::Support::Graph_ptr& graph, Iterator begin, Iterator end) {
        MiniMC::Support::getMessager().message("Generate ARG-Graph");
        auto progresser = MiniMC::Support::getMessager().makeProgresser();
        std::set<MiniMC::CPA::State_ptr> visited;
        MiniMC::Support::Stack<MiniMC::CPA::State> working;
        auto addState = [&](std::weak_ptr<MiniMC::CPA::State> winp) {
          auto inp = winp.lock();
          if (inp) {
            if (visited.count(inp) == 0) {
              working.insert(inp);
              visited.insert(inp);
            }
          }
        };

        auto insert = [&](auto& state) -> std::unique_ptr<MiniMC::Support::Node> {
          std::stringstream str;
          str << std::hash<MiniMC::CPA::State>{}(*state);
          auto node = graph->getNode(str.str());
          std::stringstream labelstr;
          labelstr << *state;
          node->setLabel(labelstr.str());
          if (state->assertViolated())
            node->color(MiniMC::Support::HSLColor(0.7, 1.0, 1.0));
          if (state->template hasLocationOf<MiniMC::Model::Attributes::ConvergencePoint>())
            node->color(MiniMC::Support::HSLColor(0.5, 1.0, 1.0));
          if (state->template hasLocationOf<MiniMC::Model::Attributes::UnrollFailed>())
            node->color(MiniMC::Support::HSLColor(0.7, 1.0, 1.0));
          if (state->template hasLocationOf<MiniMC::Model::Attributes::AssumptionPlace>())
            node->color(MiniMC::Support::HSLColor(0.3, 1.0, 1.0));

          return node;
        };

        std::for_each(begin, end, addState);
        while (!working.empty()) {
          MiniMC::Support::Localiser waitmess("Waiting: %1%");
          progresser->progressMessage(waitmess.format(working.size()));
          auto current = working.pull();
          auto curnode = insert(current);

          auto argcur = std::dynamic_pointer_cast<MiniMC::CPA::ARG::State>(current);
          assert(argcur);
          for (auto& wpar : *argcur) {
            auto state = wpar.from.lock();
            if (state) {
              auto pnode = insert(state);
              std::stringstream edgetext;
              edgetext << wpar.who << ": " << *wpar.edge;
              pnode->connect(*curnode, edgetext.str());
              addState(state);
            }
          }
        }
      }

      struct CPA : public MiniMC::CPA::ICPA {
        CPA(const MiniMC::CPA::CPA_ptr& w) : wrapped(w) {}
        virtual StateQuery_ptr makeQuery() const { return std::make_shared<StateQuery>(wrapped->makeQuery()); }
        virtual Transferer_ptr makeTransfer() const { return std::make_shared<Transferer>(wrapped->makeTransfer()); }
        virtual Joiner_ptr makeJoin() const { return std::make_shared<Joiner>(wrapped->makeJoin()); }
        virtual Storer_ptr makeStore() const { return std::make_shared<MiniMC::CPA::Storer>(makeJoin()); }
        virtual PrevalidateSetup_ptr makeValidate() const { return wrapped->makeValidate(); }

      private:
        MiniMC::CPA::CPA_ptr wrapped;
      };

    } // namespace ARG
  }   // namespace CPA
} // namespace MiniMC
