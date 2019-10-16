#include "model/cfg.hpp"
#include "model/variables.hpp"
#include "model/types.hpp"
#include "algorithms/successorgen.hpp"
#include "algorithms/passedwaiting.hpp"
#include "cpa/location.hpp"
#include "loaders/loader.hpp"



int main (int argc,char* argv[]) {
  auto loader = MiniMC::Loaders::makeLoader<MiniMC::Loaders::Type::LLVM> ();
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
  auto prgm = loader->loadFromFile (argv[1],tfac);
  
  auto initialState =  MiniMC::CPA::Location::CPADef::Query::makeInitialState (*prgm);
  std::cerr << *initialState << std::endl;
  MiniMC::Algorithms::CPADFSPassedWaiting<MiniMC::CPA::Location::CPADef> passed;
  passed.insert(initialState);
  while (passed.hasWaiting ()) {
    auto state = passed.pull ();
    MiniMC::Algorithms::Generator<MiniMC::CPA::Location::CPADef::Query,MiniMC::CPA::Location::CPADef::Transfer> gen (state);
    auto it = gen.begin();
    auto end = gen.end();
    for (;it!=end;++it) {
      std::cerr << *it->state << std::endl;
      passed.insert(it->state);
    }
  }
}
