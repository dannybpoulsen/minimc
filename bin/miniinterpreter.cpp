#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/concrete.hpp"
#include "loaders/loader.hpp"
#include "model/edge.hpp"
#include "model/output.hpp"
#include "model/types.hpp"
#include "plugin.hpp"
#include "interpreter/task.hpp"
#include "vm/concrete/concrete.hpp"
#include <boost/program_options/options_description.hpp>
#include <queue>

namespace po = boost::program_options;

void addOptions (po::options_description& op) {

}

MiniMC::Support::ExitCodes intp_main(MiniMC::Model::Controller& controller, const MiniMC::CPA::AnalysisBuilder& builder) {
  std::queue<MiniMC::Interpreter::Task*> queue;
  std::string s;
  std::unordered_map<std::string,MiniMC::CPA::AnalysisState> statemap;

  auto& prgm = *controller.getProgram ();
  auto transferer = builder.makeTransfer(prgm);
  // Build Initial state
  statemap["current"] = builder.makeInitialState(
      MiniMC::CPA::InitialiseDescr{prgm.getEntryPoints(), prgm.getHeapLayout(),
                                   prgm.getInitialiser(), prgm});

  MiniMC::Interpreter::InterpreterTaskFactory factory(&statemap, transferer);

  // Command Line
  while(true){
    std::cin >> s;
    factory.pushTask(s, &queue);
    while (!queue.empty()){
      MiniMC::Interpreter::Task* task = queue.front();
      queue.pop();
      task->performTask();
    }
  }

  return MiniMC::Support::ExitCodes::AllGood;

}

static CommandRegistrar intp_reg ("intp",intp_main,"Running the interpreter on the given configuration. ", addOptions);
