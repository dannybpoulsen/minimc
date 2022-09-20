#include "interpreter/task.hpp"
#include "cpa/interface.hpp"
#include "model/edge.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"

namespace MiniMC {
namespace Interpreter {
Task *InterpreterTaskFactory::createTask(
    std::string s, std::unordered_map<std::string, CPA::AnalysisState>* statemap,
    CPA::AnalysisTransfer transfer) {

  if (s == "printState" || s == "p") {
    return new PrintStateTask((*statemap)["current"]);
  } else if (s == "step" || s == "s") {
    return new SingleStepTask((*statemap)["current"],
                              promptForEdge((*statemap)["current"]), transfer);
  } else if(s == "bookmark") {
    return new SetBookmarkTask(statemap);
  } else if(s == "jump") {
    return new JumpToBookmarkTask(statemap);
  } else {
    return new NoMatchTask((*statemap)["current"]);
  }
}

Model::Edge *InterpreterTaskFactory::promptForEdge(CPA::AnalysisState state) {
  int index = -1;
  int n = 0;

  std::vector<Model::Edge *> edges;

  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  std::cout << "Folowing edges can be picked" << std::endl;

  // Print outgoing edges
  while (enumerator.getNext(res)) {
    edges.push_back(res.edge);
    n++;
    std::cout << n << ". " << std::endl;
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