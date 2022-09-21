#ifndef MINIMC_TASK_HPP
#define MINIMC_TASK_HPP
#include "cpa/interface.hpp"
#include "cpa/state.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include <queue>
#include <unordered_map>

namespace MiniMC {
namespace Interpreter {

class Task {
public:
  virtual void performTask() = 0;
};
class NoMatchTask : public Task {
  public:
    void performTask() {
      std::cout << "Does not recognise command"<< std::endl;
    }
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

class JumpToPreviousStateTask : public Task {
  public:
    JumpToPreviousStateTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap) : statemap(statemap){};
    void performTask(){
      (*statemap)["temp"] = (*statemap)["current"];
      (*statemap)["current"] = (*statemap)["prev"];
      (*statemap)["prev"] = (*statemap)["temp"];
    };
private:
  std::unordered_map<std::string,CPA::AnalysisState> *statemap;
};

class JumpToBookmarkTask : public Task {
  public:
    JumpToBookmarkTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap) : statemap(statemap){};
    void performTask(){
      (*statemap)["prev"] = (*statemap)["current"];
      (*statemap)["current"] = (*statemap)["bookmark"];
    };
  private:
    std::unordered_map<std::string,CPA::AnalysisState> *statemap;
};

class PrintStateTask : public Task {
public:
  PrintStateTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap, const char* state = "current" ) : statemap(statemap),state(state){};
   void performTask() {
    std::cout << (*statemap)[state];
  }

private:
  std::unordered_map<std::string,CPA::AnalysisState> *statemap;
  const char* state;
};

class SingleStepTask : public Task {
public:
  SingleStepTask(std::unordered_map<std::string,CPA::AnalysisState> *statemap,
                 const CPA::AnalysisTransfer &transfer)
      : statemap(statemap), transfer(transfer) {}
  void performTask() override {
    CPA::AnalysisState newstate;
    MiniMC::proc_t proc{0};
    (*statemap)["prev"] = (*statemap)["current"];

    if(auto noinsedge = haveNoInstructionEdge((*statemap)["current"])){
      if(transfer.Transfer((*statemap)["current"], noinsedge, proc, newstate)){
        (*statemap)["current"]=newstate;
      };
    }

    if(auto edge = promptForEdge((*statemap)["current"])){
      if(transfer.Transfer((*statemap)["current"], edge, proc, newstate)){
        (*statemap)["current"]=newstate;
      };
    } else {
      std::cout << "Current location have no outgoing edges" << std::endl;
    }
  }

private:
  std::unordered_map<std::string,CPA::AnalysisState> *statemap;
  CPA::AnalysisTransfer transfer;

  Model::Edge *promptForEdge(CPA::AnalysisState);
  Model::Edge *haveNoInstructionEdge(CPA::AnalysisState);
};

class RunPathTask : public Task {
  public:
    RunPathTask(
        std::unordered_map<std::string, CPA::AnalysisState> *statemap,
        const CPA::AnalysisTransfer &transfer, std::vector<int>* indexes)
        : statemap(statemap), transfer(transfer), indexes(indexes) {}

    void performTask(){
      CPA::AnalysisState newstate;
      MiniMC::proc_t proc{0};

      std::for_each((*indexes).begin(), (*indexes).end(), [&](const int i){
        Algorithms::EdgeEnumerator enumerator{(*statemap)["current"]};
        Algorithms::EnumResult res;
        for(int j = 0; j < i; j++){
          enumerator.getNext(res);
        }

        if(transfer.Transfer((*statemap)["current"], res.edge, proc, newstate)){
          (*statemap)["current"]=newstate;
        };
      });
    }


  private:
    std::unordered_map<std::string,CPA::AnalysisState> *statemap;
    CPA::AnalysisTransfer transfer;
    std::vector<int>* indexes;
};

class TaskFactory {
public:
  virtual void pushTask(std::string,
                          std::queue<Task *> *) = 0;
};

class InterpreterTaskFactory : public TaskFactory {
  public:
    InterpreterTaskFactory(std::unordered_map<std::string, CPA::AnalysisState>* statemap, CPA::AnalysisTransfer transfer);
    void pushTask(std::string, std::queue<Task *> *) override;
    void queueRun(std::vector<int>*, std::queue<Task*>*);
  private:
    std::unordered_map<std::string,std::vector<Task*>> commands;
    std::unordered_map<std::string, CPA::AnalysisState>* statemap;
    CPA::AnalysisTransfer transfer;
};

} // namespace Interpreter
} // namespace MiniMC
#endif // MINIMC_TASK_HPP
