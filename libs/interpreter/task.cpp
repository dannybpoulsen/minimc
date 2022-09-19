#include "interpreter/task.hpp"
#include "cpa/interface.hpp"
#include "model/edge.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"

namespace MiniMC {
namespace Interpreter {
class PrintStateTask : public Task {
  public:
    PrintStateTask(MiniMC::CPA::AnalysisState* state ){this->state =state;};
    MiniMC::CPA::AnalysisState* performTask(){
      std::cout << this->state;
      delete this;
      return state;
    }
  private:
    MiniMC::CPA::AnalysisState* state;

};

  class InterpreterTaskFactory : public TaskFactory {
    public:
      Task* createTask(std::string s,MiniMC::CPA::AnalysisState* state){
        if(s == "printState" && "p"){
          return new PrintStateTask(state);
        }
        return nullptr;
      }
  };

} // namespace Interpreter
} // namespace MiniMC