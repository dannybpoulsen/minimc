#include "task.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/interface.hpp"
#include "model/edge.hpp"
#include <queue>

namespace MiniMC {
namespace Interpreter {
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

  // Print outgoing edges
  if (enumerator.getNext(res)) {
    std::cout << "Following edges can be picked" << std::endl;
    edges.push_back(res.edge);
    n++;
    std::cout << n << ". " << std::endl;
    std::cout << *res.edge;
    while (enumerator.getNext(res)) {
      edges.push_back(res.edge);
      n++;
      std::cout << n << ". " << std::endl;
      std::cout << *res.edge;
    }
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

void InterpreterTaskFactory::pushTask(
    std::string  s,
    std::queue<Task*>* queue) {
  std::vector<Task*> tasks;
  if (commands.contains(s)){
    tasks = commands[s];
  } else {
    tasks = commands["nomatch"];
  }
  std::for_each(tasks.begin(), tasks.end(), [queue](auto& task) { queue->push(task); });
}

InterpreterTaskFactory::InterpreterTaskFactory(std::unordered_map<std::string, CPA::AnalysisState> *statemap, CPA::AnalysisTransfer transfer) :statemap(statemap),transfer(transfer) {
  auto printcurrent = new PrintStateTask(statemap);
  auto printbookmark = new PrintStateTask(statemap, "bookmark");
  auto setbookmark = new SetBookmarkTask(statemap);
  auto jumpbookmark = new JumpToBookmarkTask(statemap);
  auto jumpprevious = new JumpToPreviousStateTask(statemap);
  auto singlestep = new SingleStepTask(statemap, transfer);
  auto nomatch = new NoMatchTask();

  // Print state
  commands["print"] = {printcurrent};
  commands["p"] = {printcurrent};
  commands["pb"] = {printbookmark};

  // Single step
  commands["step"] = {singlestep, printcurrent};
  commands["s"] = {singlestep, printcurrent};

  // Bookmarks
  commands["bookmark"] = {setbookmark,printcurrent};
  commands["b"] = {setbookmark};

  commands["jump"] = {jumpbookmark, printcurrent};
  commands["jb"] = {jumpbookmark, printcurrent};
  commands["prev"] = {jumpprevious, printcurrent};

  // Default case
  commands["nomatch"] = {nomatch};
}

void InterpreterTaskFactory::queueRun(std::vector<int> *path, std::queue<Task*>* queue) {
  queue->push(new RunPathTask(statemap,transfer,path));
}

}// namespace Interpreter
} // namespace MiniMC
