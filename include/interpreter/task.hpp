#ifndef MINIMC_TASK_HPP
#define MINIMC_TASK_HPP
#include "cpa/state.hpp"
namespace MiniMC {
  namespace Interpreter {
    enum TaskEnum {
      singlestep,
      printstate
    };

  class Task {
    public:
      virtual ~Task(){};
      virtual MiniMC::CPA::AnalysisState performTask() const = 0;
    };

    class TaskFactory {
    public:
      virtual ~TaskFactory(){};
      virtual Task getTask(TaskEnum);
    };

  } // namespace Interpreter
} // namespace MiniMC
#endif // MINIMC_TASK_HPP
