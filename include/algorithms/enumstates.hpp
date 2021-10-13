#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include "algorithms/algorithm.hpp"
#include "algorithms/reachability.hpp"
#include "algorithms/simulationmanager.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/interface.hpp"
#include "support/exceptions.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include <sstream>

namespace MiniMC {
  namespace Algorithms {
    class EnumStates : public MiniMC::Algorithms::Algorithm {
    public:
      struct Options {
        MiniMC::CPA::CPA_ptr cpa;
      };

      EnumStates(const Options& opt) : messager(MiniMC::Support::getMessager()), cpa(opt.cpa) {}
      virtual Result run(const MiniMC::Model::Program& prgm) {
        if (!cpa->makeValidate()->validate(prgm, messager)) {
          return Result::Error;
        }
        messager.message("Initiating EnumStates");

        auto progresser = messager.makeProgresser();
        auto predicate = [](auto& b) { return false; };
        auto query = cpa->makeQuery();
        auto transfer = cpa->makeTransfer();
        auto initstate = query->makeInitialState(prgm);

        MiniMC::Algorithms::SimulationManager simmanager(MiniMC::Algorithms::SimManagerOptions{
            .storer = cpa->makeStore(),
            .transfer = cpa->makeTransfer()});
        simmanager.insert(initstate);
        simmanager.reachabilitySearch({.filter = [](const MiniMC::CPA::State_ptr& state) {
          return state->getConcretizer()->isFeasible() == MiniMC::CPA::Concretizer::Feasibility::Feasible;
        }});

        messager.message("Finished EnumStates");
        messager.message(MiniMC::Support::Localiser("Total Number of States %1%").format(simmanager.getPSize()));
        return Result::Success;
      }

    private:
      MiniMC::Support::Messager& messager;
      MiniMC::CPA::CPA_ptr cpa;
    };
  } // namespace Algorithms
} // namespace MiniMC

#endif
