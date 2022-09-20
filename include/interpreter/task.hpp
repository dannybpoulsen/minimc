#ifndef MINIMC_TASK_HPP
#define MINIMC_TASK_HPP
#include "cpa/state.hpp"
#include "cpa/interface.hpp"

namespace MiniMC {
namespace Interpreter {

class Task {
public:
  virtual MiniMC::CPA::AnalysisState performTask() = 0;
};

class PrintStateTask : public Task {
public:
  PrintStateTask(MiniMC::CPA::AnalysisState state) : state(state){};
  CPA::AnalysisState performTask() {
    std::cout << this->state;
    return state;
  }

private:
  MiniMC::CPA::AnalysisState state;
};

class SingleStepTask : public Task {
public:
  SingleStepTask(CPA::AnalysisState state, Model::Edge *edge,
                 const CPA::AnalysisTransfer &transfer)
      : state(state), edge(edge), transfer(transfer) {}
  CPA::AnalysisState performTask() override {
    CPA::AnalysisState newstate;
    MiniMC::proc_t proc{0};

    if(transfer.Transfer(state, edge, proc, newstate)){
      return newstate;
    };

    return state;
  }

private:
  CPA::AnalysisState state;
  Model::Edge *edge;
  CPA::AnalysisTransfer transfer;
};

class TaskFactory {
public:
  virtual Task* createTask(std::string,std::unordered_map<std::string, CPA::AnalysisState> statemap,MiniMC::CPA::AnalysisTransfer) = 0;
};

class InterpreterTaskFactory : public TaskFactory{
public:
  Task *createTask(std::string string,
                   std::unordered_map<std::string,CPA::AnalysisState> statemap, MiniMC::CPA::AnalysisTransfer transfer) override;
private:
  Model::Edge *promptForEdge(CPA::AnalysisState state);
};

} // namespace Interpreter
} // namespace MiniMC
#endif // MINIMC_TASK_HPP
