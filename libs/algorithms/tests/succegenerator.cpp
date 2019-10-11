#include "model/cfg.hpp"
#include "model/variables.hpp"
#include "model/types.hpp"
#include "algorithms/successorgen.hpp"
#include "cpa/location.hpp"


int main () {
  MiniMC::Model::TypeFactory64 tfactory;
  MiniMC::Model::Program prgm;
  auto i8 = tfactory.makeIntegerType(8);
  auto globals = prgm.makeVariableStack ();
  auto res = globals->addVariable ("Res",i8);
  auto v1 = globals->addVariable ("var1",i8);
  auto v2 = globals->addVariable ("var2",i8);
  
  auto cfg = std::make_shared<MiniMC::Model::CFG > ();
  auto init = cfg->makeLocation ("Initial");
  cfg->setInitial (init);
  auto ss = cfg->makeLocation ("Succ");
  auto ss3 = cfg->makeLocation ("Succ2");
  std::vector<MiniMC::Model::Instruction> inst ;
  cfg->makeEdge (init,ss,inst,nullptr);
  cfg->makeEdge (init,ss3,inst,nullptr);
  std::vector<gsl::not_null<MiniMC::Model::Variable_ptr>> params;
  auto f = prgm.addFunction ("Main",params,globals,gsl::not_null<MiniMC::Model::CFG_ptr>(cfg));
  prgm.addEntryPoint (f);
  prgm.addEntryPoint (f);
  
  
  auto initialState =  MiniMC::CPA::Location::CPADef::Query::makeInitialState (prgm);
  std::cerr << *initialState << std::endl;
  MiniMC::Algorithms::Generator<MiniMC::CPA::Location::CPADef::Query,MiniMC::CPA::Location::CPADef::Transfer> gen (initialState);
  auto it = gen.begin();
  auto end = gen.end();
  for (;it!=end;++it) {
    std::cerr << *it->state << std::endl;
  }
}
