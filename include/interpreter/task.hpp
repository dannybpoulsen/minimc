#ifndef MINIMC_TASK_HPP
#define MINIMC_TASK_HPP
#include "cpa/interface.hpp"
#include "cpa/state.hpp"
#include <queue>

namespace MiniMC {
namespace Interpreter {

class Task {
public:
  virtual void performTask() = 0;
};
class NoMatchTask : public Task {
  public:
    NoMatchTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap) : statemap(statemap){};
    void performTask() {
      std::cout << "Does not recognise command"<< std::endl;
    }

private:
  std::unordered_map<std::string,CPA::AnalysisState> *statemap;
};

class SetBookmarkTask : public Task {
  public:
    SetBookmarkTask(std::unordered_map<std::string,CPA::AnalysisState>* statemap) : statemap(statemap){};
    void performTask(){
      (*statemap)["bookmark"] = (*statemap)["current"];
    };

  private:
    std::unordered_map<std::string,CPA::AnalysisState> *statemap;
};

class JumpToBookmarkTask : public Task {
  public:
    JumpToBookmarkTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap) : statemap(statemap){};
    void performTask(){
      (*statemap)["current"] = (*statemap)["bookmark"];
    };
  private:
    std::unordered_map<std::string,CPA::AnalysisState> *statemap;
};

class PrintStateTask : public Task {
public:
  PrintStateTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap) : statemap(statemap){};
   void performTask() {
    std::cout << (*statemap)["current"];
  }

private:
  std::unordered_map<std::string,CPA::AnalysisState> *statemap;
};

class SingleStepTask : public Task {
public:
  SingleStepTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap,
                 const CPA::AnalysisTransfer &transfer)
      : statemap(statemap), transfer(transfer) {}
  void performTask() override {
    CPA::AnalysisState newstate;
    MiniMC::proc_t proc{0};

    if(auto edge = haveNoInstructionEdge((*statemap)["current"])){
      if(transfer.Transfer((*statemap)["current"], edge, proc, newstate)){
        (*statemap)["current"]=newstate;
      };
    }
    if(transfer.Transfer((*statemap)["current"], promptForEdge((*statemap)["current"]), proc, newstate)){
      (*statemap)["current"]=newstate;
    };


  }

private:
  std::unordered_map<std::string,CPA::AnalysisState> *statemap;
  CPA::AnalysisTransfer transfer;

  Model::Edge *promptForEdge(CPA::AnalysisState);
  Model::Edge *haveNoInstructionEdge(CPA::AnalysisState);
};

class TaskFactory {
public:
  virtual void pushTask(std::string,
                          std::unordered_map<std::string, CPA::AnalysisState> *,
                          MiniMC::CPA::AnalysisTransfer,
                          std::queue<Task *> *) = 0;
};

class InterpreterTaskFactory : public TaskFactory {
public:
  void pushTask(std::string,
                  std::unordered_map<std::string, CPA::AnalysisState> *,
                  MiniMC::CPA::AnalysisTransfer, std::queue<Task *> *) override;

};

} // namespace Interpreter
} // namespace MiniMC
#endif // MINIMC_TASK_HPP
