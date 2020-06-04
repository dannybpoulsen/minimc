#include <boost/program_options.hpp>

#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/statistical/fixed_effort.hpp"
#include "algorithms/reach.hpp"

#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

auto runAlgorithm (MiniMC::Model::Program& prgm,  const MiniMC::Algorithms::SetupOptions sopt) {
  using algorithm = MiniMC::Algorithms::ExplicitReachability;;
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm<algorithm> (seq,sopt);
  algorithm algo(typename algorithm::Options {.messager = sopt.messager});
  return MiniMC::Algorithms::runSetup (seq,algo,prgm);
}


int mc_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters,  MiniMC::Algorithms::SetupOptions& sopt) {
  sopt.expandNonDet = true;
  runAlgorithm (*prgm,sopt);
  return 0;
}

static CommandRegistrar mc_reg ("mc",mc_main,"Check whether it is possible to reach an assert violation. Classic reachability analysis. ");
