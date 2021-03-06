#include <boost/program_options.hpp>

#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "algorithms/enumstates.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/replacememnondet.hpp"
#include "cpa/location.hpp"
#include "cpa/concrete_no_mem.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/compound.hpp"


#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

namespace {

template<class CPADef>
auto runAlgorithm (MiniMC::Model::Program& prgm, const MiniMC::Algorithms::SetupOptions sopt) {
  using algorithm = MiniMC::Algorithms::EnumStates<CPADef>;
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm<algorithm> (seq,sopt);
  algorithm algo(typename algorithm::Options {.messager = sopt.messager});
  return MiniMC::Algorithms::runSetup (seq,algo,prgm);
}

enum class CPAUsage {
					 Location,
					 LocationExplicit,
					 CVC4PathFormula
};
}

int enum_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters,  const MiniMC::Algorithms::SetupOptions& sopt)  {
    CPAUsage CPA = CPAUsage::Location;
	MiniMC::Algorithms::SpaceReduction reduction;
  int SpaceReduction = 0;
  po::options_description desc("Print Graph Options");
  std::string input;
  auto updateCPA = [&CPA] (int val) {
					 switch (val) {
					 case 2:
					   CPA = CPAUsage::LocationExplicit;
					   break;
					 case 3:
					   CPA = CPAUsage::CVC4PathFormula;
					   break;
					 case 1:
					 default:
					   CPA = CPAUsage::Location;
					   break;
					 
					 }
				   };
  
  desc.add_options()
    ("cpa,c",po::value<int>()->default_value(1)->notifier(updateCPA), "CPA\n"
     "\t 1: Location\n"
     "\t 2: Location and explicit stack-variable\n"
	 "\t 3: PathFormula With CVC4\n"
     );
    
  
  po::variables_map vm; 
    if (!parseOptionsAddHelp (vm,desc,parameters)) {
	return -1;
  }
	  
  using LocExpliStack = MiniMC::CPA::Compounds::CPADef<0,
													   MiniMC::CPA::Location::CPADef,
													   MiniMC::CPA::ConcreteNoMem::CPADef
													   >;
  using CVC4Path = MiniMC::CPA::Compounds::CPADef<0,
												  MiniMC::CPA::Location::CPADef,
												  MiniMC::CPA::PathFormula::CVC4CPA
												  >;
  MiniMC::Algorithms::Result res;
  switch (CPA) {
  case CPAUsage::CVC4PathFormula:
	res = runAlgorithm<CVC4Path> (*prgm,sopt);
	break;
  case CPAUsage::LocationExplicit:
    res = runAlgorithm<LocExpliStack> (*prgm,sopt);
    break;
  case CPAUsage::Location:
  default:
    res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt);
    break;
  }
  
  return static_cast<int> (res);
  
}

static CommandRegistrar enum_reg ("enum",enum_main,"Enumerate total number of states in CPA");

