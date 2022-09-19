#ifndef MINIMC_TASK_HPP
#define MINIMC_TASK_HPP
#include "cpa/state.hpp"
#include "cpa/interface.hpp"

namespace MiniMC {
namespace Interpreter {

class Task {
public:
  virtual ~Task();
  virtual MiniMC::CPA::AnalysisState* performTask() = 0;
};

class TaskFactory {
public:
  virtual ~TaskFactory();
  virtual Task* createTask(std::string,MiniMC::CPA::AnalysisState*) = 0;
};

} // namespace Interpreter
} // namespace MiniMC
#endif // MINIMC_TASK_HPP
