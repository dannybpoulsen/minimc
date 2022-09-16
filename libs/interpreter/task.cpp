#include "interpreter/task.hpp"
#include "cpa/interface.hpp"
#include "model/edge.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"

namespace MiniMC {
  namespace Interpreter {

    class SingleStepTask : public Task {
      public:
        SingleStepTask(MiniMC::CPA::AnalysisState state,
                       MiniMC::Model::Edge* edge,
                       MiniMC::CPA::AnalysisTransfer* transferer) {
          state = state;
          edge = edge;
          transferer = transferer;
        }

        MiniMC::CPA::AnalysisState performTask() const override {
          MiniMC::proc_t proc{0};
          MiniMC::CPA::AnalysisState newstate;
          transferer->Transfer(state, edge, proc, newstate);
          return newstate;
        }
      private:
        MiniMC::CPA::AnalysisState state;
        MiniMC::Model::Edge* edge;
        MiniMC::CPA::AnalysisTransfer* transferer;
    };

    class PrintStateTask : public Task {
    public:
      PrintStateTask(MiniMC::CPA::AnalysisState state){
        state = state;
      }
      MiniMC::CPA::AnalysisState performTask() const override{
        std::cout << state;
        return state;
      }
    private:
      MiniMC::CPA::AnalysisState state;
    };

    class InterpreterTaskFactory : public TaskFactory {
    public:
      Task getTask(TaskEnum task) override {
        switch (task) {
        case TaskEnum::singlestep:
          SingleStepTask rtask = SingleStepTask(state,promptForEdge(),transferer);
          break;
        case TaskEnum::printstate:
          PrintStateTask rtask = PrintStateTask(state);
          break;
        }
        return rtask;
      }
    private:
      MiniMC::CPA::AnalysisState state;
      MiniMC::CPA::AnalysisTransfer* transferer;

      MiniMC::Model::Edge* promptForEdge(){
        int index;
        int n = 0;

        std::vector<MiniMC::Model::Edge*> edges;

        MiniMC::Algorithms::EdgeEnumerator enumerator{state};
        MiniMC::Algorithms::EnumResult res;

        os << "Folowing edges can be picked" << std::endl ;

        // Print outgoing edges
        while (enumerator.getNext(res)) {
          edges.push_back(res.edge);
          n++;
          os << n << ". "<< std::endl;
        }

        // Choose edge by index
        while (index <= 0) {
          std::cin >> index;
          if(edges[index-1]){
            return edges[index-1];
          }
          os << "The chosen index is not possible, please choose a index under "<< edges.size();
        }
      }
    };
  } // namespace Interpreter
} // namespace MiniMC