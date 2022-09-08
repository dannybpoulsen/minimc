
#include "algorithms/successorgen.hpp"
#include "cpa/state.hpp"
#include "model/edge.hpp"
#include <vector>


MiniMC::Model::Edge *promptForEdge(MiniMC::CPA::AnalysisState state) {
  int index = -1;

  std::vector<MiniMC::Model::Edge*> edges;

  std::cout << "Folowing edges can be picked" << std::endl ;
  MiniMC::Algorithms::EdgeEnumerator enumerator{state};
  MiniMC::Algorithms::EnumResult res;

  while (enumerator.getNext(res)) {
    std::cout << " -->";
    std::cout << res.edge->getTo()->getInfo().getName() << std::endl;
    edges.push_back(res.edge);
  }

  while ((0 >= index && index > edges.size())) {
    std::cin >> index;
  }

  return edges[index-1];

};

void printState(MiniMC::CPA::AnalysisState state){
    state.getCFAState()->output(std::cout);

}
