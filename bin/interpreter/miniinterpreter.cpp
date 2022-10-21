#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/concrete.hpp"
#include "model/edge.hpp"
#include "model/output.hpp"
#include "model/types.hpp"
#include "plugin.hpp"
#include "task.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "vm/concrete/concrete.hpp"
#include <boost/program_options/options_description.hpp>
#include <queue>
#include <utility>

namespace po = boost::program_options;

struct LocalOptions {
  std::string path{""};
};

LocalOptions locoptions;

void addOptions (po::options_description& op) {
  auto setPath = [&](std::string val) { locoptions.path = std::move(val); };

  po::options_description desc("MC Options");
  desc.add_options()("intp.path",
                     po::value<std::string>()->default_value("")->notifier(setPath),
                     "The indexes of the edges needed to form a path");

  op.add(desc);
}

MiniMC::Host::ExitCodes intp_main(MiniMC::Model::Controller& controller, const MiniMC::CPA::AnalysisBuilder& builder) {
  std::queue<MiniMC::Interpreter::Task*> queue;
  char* tok;
  char delim[] = " ";
  std::string s;
  std::vector<int> path;

  if(locoptions.path != ""){
    tok = std::strtok(const_cast<char *>(locoptions.path.c_str()), delim);
    while(tok != NULL){
      path.push_back(std::stoi(tok));
      tok = std::strtok(NULL,delim);
    }
  }

  auto& prgm = *controller.getProgram ();
  auto transferer = builder.makeTransfer(prgm);
  // Build Initial state
  MiniMC::Interpreter::StateMap statemap(builder.makeInitialState(
      MiniMC::CPA::InitialiseDescr{prgm.getEntryPoints(), prgm.getHeapLayout(),
                                   prgm.getInitialiser(), prgm}));

  MiniMC::Interpreter::Parser parser(&statemap, transferer);


  // Command Line
  while(true){
    std::getline(std::cin,s);
    if(!std::cin || s == "quit") break;
    parser(s);
  }

  return MiniMC::Host::ExitCodes::AllGood;

}

static CommandRegistrar intp_reg ("intp",intp_main,"Running the interpreter on the given configuration. ", addOptions);
