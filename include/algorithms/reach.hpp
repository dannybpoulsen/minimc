#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include "algorithms/algorithm.hpp"
#include "algorithms/simulationmanager.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/compound.hpp"
#include "cpa/concrete.hpp"
#include "cpa/location.hpp"
#include "support/exceptions.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include <functional>
#include <sstream>

namespace MiniMC {
  namespace Algorithms {
    class Reachability : public MiniMC::Algorithms::Algorithm {
    public:
      enum class ReachabilityResult {
        Found,
        NotFound,
        Inconclusive
      };

      struct AnalysisResult {
        ReachabilityResult result;
        MiniMC::CPA::State_ptr foundState;
      };

      struct Options {
        MiniMC::Algorithms::GoalFunction predicate = [](const MiniMC::CPA::State_ptr& state) { return state->assertViolated(); };
        MiniMC::Algorithms::FilterFunction filter = [](const MiniMC::CPA::State_ptr& state) {
          return state->getConcretizer()->isFeasible() == MiniMC::CPA::Concretizer::Feasibility::Feasible;
        };

        MiniMC::CPA::CPA_ptr cpa = nullptr;
      };
      Reachability(const Options& opt) : messager(MiniMC::Support::getMessager()),
                                         predicate(opt.predicate),
                                         filter(opt.filter),
                                         cpa(opt.cpa) {}

      virtual Result run(const MiniMC::Model::Program& prgm) {
        if (!cpa->makeValidate()->validate(prgm, messager)) {
          return Result::Error;
        }

        messager.message("Initiating Reachability");
        auto query = cpa->makeQuery();
        auto transfer = cpa->makeTransfer();
        MiniMC::CPA::State_ptr foundState = nullptr;

        auto progresser = messager.makeProgresser();

        auto initstate = query->makeInitialState(prgm);
        MiniMC::Algorithms::SimulationManager simmanager(MiniMC::Algorithms::SimManagerOptions{
            .storer = cpa->makeStore(),
            .transfer = cpa->makeTransfer()});
        simmanager.insert(initstate);
        foundState = simmanager.reachabilitySearch({.filter = filter,
                                                    .goal = predicate

        });
       
        messager.message("Finished Reachability");
        if (foundState) {
          result.result = ReachabilityResult::Found;
          result.foundState = foundState;
          return Result::Success;
        } else {
          result.result = ReachabilityResult::NotFound;
          return Result::Success;
        }
      }

      const auto& getAnalysisResult() const { return result; }

    private:
      MiniMC::Support::Messager& messager;
      AnalysisResult result{.result = ReachabilityResult::Inconclusive};
      MiniMC::Algorithms::GoalFunction predicate;
      MiniMC::Algorithms::FilterFunction filter;
      ;
      MiniMC::CPA::CPA_ptr cpa;
    };
  } // namespace Algorithms
} // namespace MiniMC

#endif
