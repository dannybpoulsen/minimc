#include "interpreter/task.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/interface.hpp"
#include "model/edge.hpp"
#include <queue>

namespace MiniMC {
namespace Interpreter {
void InterpreterTaskFactory::pushTask(
    std::string  s,
    std::unordered_map<std::string, CPA::AnalysisState> *statemap,
    CPA::AnalysisTransfer transfer,
    std::queue<Task*>* queue) {

  if (s == "printState" || s == "p") {
    queue->push(new PrintStateTask(statemap));
  } else if (s == "step" || s == "s") {
    queue->push(new SingleStepTask(statemap, transfer));
  } else if (s == "bookmark") {
    queue->push(new SetBookmarkTask(statemap));
  } else if (s == "jump") {
    queue->push(new JumpToBookmarkTask(statemap));
  } else {
    queue->push(new NoMatchTask(statemap));
  }
}
Model::Edge* SingleStepTask::haveNoInstructionEdge(CPA::AnalysisState state) {
  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  while (enumerator.getNext(res)) {
    if(!res.edge->getInstructions()) return res.edge;
  }
  return nullptr;
}

Model::Edge *SingleStepTask::promptForEdge(CPA::AnalysisState state) {
  int index = -1;
  int n = 0;

  std::vector<Model::Edge *> edges;

  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  std::cout << "Following edges can be picked" << std::endl;

  // Print outgoing edges
  while (enumerator.getNext(res)) {
    edges.push_back(res.edge);
    n++;
    std::cout << n << ". " << std::endl;
    std::cout << *res.edge;
  }

  if (edges.size() == 0) {
    return nullptr;
  }

  // Choose edge by index
  while (index <= 0) {
    std::cin >> index;
    if (edges[index - 1]) {
      return edges[index - 1];
    }
    std::cout
        << "The chosen index is not possible, please choose a index under "
        << edges.size();
  }
  return nullptr;
};

}// namespace Interpreter
} // namespace MiniMC