#include "model/types.hpp"
#include "support/feedback.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/successorgen.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/printgraph.hpp"
#include "cpa/location.hpp"
#include "loaders/loader.hpp"



int main (int argc,char* argv[]) {
  auto loader = MiniMC::Loaders::makeLoader<MiniMC::Loaders::Type::LLVM> ();
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
  auto prgm = loader->loadFromFile (argv[1],tfac);
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);
  MiniMC::Algorithms::PrintCPA<MiniMC::CPA::Location::CPADef> algorithm (*mess);
  algorithm.run (*prgm);
  
}
