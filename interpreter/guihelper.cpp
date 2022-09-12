
#include "algorithms/algorithms.hpp"
#include "algorithms/reachability/reachability.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/concrete.hpp"
#include "cpa/interface.hpp"
#include "cpa/state.hpp"
#include "model/edge.hpp"
#include "model/output.hpp"
#include "storage/storage.hpp"

#include <vector>


MiniMC::Model::Edge *promptForEdge(MiniMC::CPA::AnalysisState state) {
  int index = -1;
  int n = 0;

  std::vector<MiniMC::Model::Edge*> edges;

  std::cout << "Folowing edges can be picked" << std::endl ;
  MiniMC::Algorithms::EdgeEnumerator enumerator{state};
  MiniMC::Algorithms::EnumResult res;

  // Print outgoing edges
  while (enumerator.getNext(res)) {
    n++;
    std::cout << n <<". ";
    std::cout << *res.edge;
    std::cout << std::endl;
    edges.push_back(res.edge);
  }

  // Choose edge by index
  while ((0 >= index && index > edges.size())) {
    std::cin >> index;
  }

  return edges[index-1];

};