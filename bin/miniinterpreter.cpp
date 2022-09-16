#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/concrete.hpp"
#include "loaders/loader.hpp"
#include "model/edge.hpp"
#include "model/output.hpp"
#include "model/types.hpp"
#include "plugin.hpp"
#include "vm/concrete/concrete.hpp"
#include <boost/program_options/options_description.hpp>

namespace po = boost::program_options;

MiniMC::Model::Edge *promptForEdge(MiniMC::CPA::AnalysisState state, int lookahead, std::ostream& os);
void stepThroughCFG(MiniMC::CPA::AnalysisState initialstate, MiniMC::CPA::AnalysisTransfer transferer, int lookahead, std::ostream& os);
void doLookAhead(MiniMC::Model::Edge* edge, int lookahead, std::ostream& os);
void readInput();
void doTask();

void addOptions (po::options_description& op) {

}

MiniMC::Support::ExitCodes intp_main(MiniMC::Model::Controller& controller, const MiniMC::CPA::AnalysisBuilder& builder) {
  int lookahead = 1;
  auto& prgm = *controller.getProgram ();
  auto transferer = builder.makeTransfer(prgm);
  // Build Initial state
  auto initialstate = builder.makeInitialState(
      MiniMC::CPA::InitialiseDescr{prgm.getEntryPoints(), prgm.getHeapLayout(),
                                   prgm.getInitialiser(), prgm});

  stepThroughCFG(initialstate, transferer, lookahead, std::cout);
  return MiniMC::Support::ExitCodes::AllGood;

}

void stepThroughCFG(MiniMC::CPA::AnalysisState initialstate,
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
    if(!edge){
      os << "Seems like there is no outgoing edges.";
      return ;
    }
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

  if(edges.size() == 0 ){
    return 0;
  }

  // Choose edge by index
  while (index <= 0) {
    std::cin >> index;
    if(edges[index-1]){
      return edges[index-1];
    }
    os << "The chosen index is not possible, please choose a index under "<< edges.size();
  }
  return 0;
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

static CommandRegistrar intp_reg ("intp",intp_main,"Running the interpreter on the given configuration. ", addOptions);
