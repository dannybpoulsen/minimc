#include "../bin/plugin.hpp"
#include "cpa/concrete.hpp"
#include "loaders/loader.hpp"
#include "model/edge.hpp"
#include "model/output.hpp"
#include "model/types.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "vm/concrete/concrete.hpp"
#include <boost/program_options/options_description.hpp>

namespace po = boost::program_options;

MiniMC::Model::Edge *promptForEdge(MiniMC::CPA::AnalysisState state);

int main(int argc, char *argv[]) {

  // Load program from file
  std::string inputname;
  std::string task("main");
  if (argc) {
    inputname = argv[1];
  }

  // Initiate Program
  MiniMC::Model::TypeFactory_ptr tfac =
      std::make_shared<MiniMC::Model::TypeFactory64>();
  MiniMC::Model::ConstantFactory_ptr cfac =
      std::make_shared<MiniMC::Model::ConstantFactory64>(tfac);
  auto loader = MiniMC::Loaders::getLoaders()[0]->makeLoader(tfac, cfac);
  MiniMC::Loaders::LoadResult loadresult = loader->loadFromFile(inputname);

  MiniMC::Model::Controller control(*loadresult.program,
                                    loadresult.entrycreator);
  control.addEntryPoint(task,{});

  MiniMC::Model::Program &prgm = *control.getProgram();

  // AnalysisBuilder
  MiniMC::CPA::AnalysisBuilder builder{std::make_shared<MiniMC::CPA::Location::CPA> ()};
  builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA>());

  auto transferer = builder.makeTransfer(prgm);

  // Build Initial state
  auto initialstate = builder.makeInitialState(
      MiniMC::CPA::InitialiseDescr{prgm.getEntryPoints(), prgm.getHeapLayout(),
                                   prgm.getInitialiser(), prgm});

  std::list<MiniMC::CPA::AnalysisState> waiting;

  MiniMC::proc_t proc{0};

  waiting.push_back(initialstate);

  // Step through the CFG
  while (waiting.size ()) {

    auto state = std::move(waiting.back());
    waiting.pop_back();

    std::cout << state;

    MiniMC::CPA::AnalysisState newstate;

    // Choose edge
    MiniMC::Model::Edge* edge = promptForEdge(state);

    // Process chosen edge
    if (transferer.Transfer(state, edge, proc, newstate)) {
      waiting.push_front(newstate);
    }
  }

  std::cout << "Reached the end";
}

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

