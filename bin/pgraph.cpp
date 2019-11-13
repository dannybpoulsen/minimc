#include "support/feedback.hpp"
#include "algorithms/printgraph.hpp"
#include "cpa/location.hpp"
#include "cpa/concrete_no_mem.hpp"
#include "cpa/compound.hpp"


#include "loaders/loader.hpp"



int main (int argc,char* argv[]) {
  auto loader = MiniMC::Loaders::makeLoader<MiniMC::Loaders::Type::LLVM> ();
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
  auto prgm = loader->loadFromFile (argv[1],tfac);
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);
  using CPADef = MiniMC::CPA::Compounds::CPADef<0,
						MiniMC::CPA::Location::CPADef
						>;
  MiniMC::Algorithms::PrintCPA<CPADef> algorithm (*mess);
  algorithm.run (*prgm);
}
