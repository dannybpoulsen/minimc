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

MiniMC::Model::Edge *promptForEdge(MiniMC::CPA::AnalysisState state, int lookahead, std::ostream& os);
void traverseCFG(MiniMC::CPA::AnalysisState initialstate, MiniMC::CPA::AnalysisTransfer transferer, int lookahead, std::ostream& os);
void doLookAhead(MiniMC::Model::Edge* edge, int lookahead, std::ostream& os);


int main(int argc, char *argv[]) {

  // Load program from file
  std::string inputname;
  std::string task("main");
  int lookahead = 1;
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

  control.boolCasts();
  control.createAssertViolateLocations();
  if (!control.typecheck ()) {
    return -1;
  }
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


  traverseCFG(initialstate,transferer, lookahead, std::cout);

}

void traverseCFG(MiniMC::CPA::AnalysisState initialstate,
                  MiniMC::CPA::AnalysisTransfer transferer, int lookahead,
                  std::ostream &os) {
  std::list<MiniMC::CPA::AnalysisState> waiting;
  MiniMC::proc_t proc{0};
  waiting.push_back(initialstate);

  while(waiting.size ()){
    MiniMC::CPA::AnalysisState newstate;
    auto state = std::move(waiting.back());
    waiting.pop_back();


    os << state;
    MiniMC::Model::Edge* edge = promptForEdge(state,lookahead,os);
    if(transferer.Transfer(state,edge,proc,newstate)) {
      waiting.push_front(newstate);
    }
  }
}

MiniMC::Model::Edge *promptForEdge(MiniMC::CPA::AnalysisState state, int lookahead, std::ostream& os) {
  int index = -1;
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
    doLookAhead(res.edge, lookahead, os);
  }

  // Choose edge by index
  while ((0 >= index && index > edges.size())) {
    std::cin >> index;
  }

  return edges[index-1];

};

void doLookAhead(MiniMC::Model::Edge* edge, int lookahead, std::ostream& os){
  if(lookahead == 0){
    return;
  }
  os << *edge;

  for (auto it = edge->getTo()->ebegin(); it !=  edge->getTo()->eend(); ++it) {
    doLookAhead(*it, lookahead-1,os);
  }
}

