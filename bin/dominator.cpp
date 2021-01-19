#include <boost/program_options.hpp>
#include <string>
#include <vector>


#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/graph.hpp"
#include "algorithms/printgraph.hpp"
#include "savers/savers.hpp"

#include "cpa/location.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/compound.hpp"

#include "model/analysis/dominator.hpp"
#include "model/analysis/loops.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;
namespace {

  void createDomTree (const std::string& name, const MiniMC::Model::Analysis::DominatorInfo<MiniMC::Model::Location>& dom, const MiniMC::Model::CFG& cfg) {
	auto graph = MiniMC::Support::CreateGraph<MiniMC::Support::GraphType::DOT> (name);
	for (auto& loc : cfg.getLocations ()) {
	  auto gnode = graph->getNode (loc->getName());
	  for (auto it = dom.dombegin (loc); it != dom.domend (loc);++it) {
		auto dominatnode = graph->getNode ((*it)->getName());
		dominatnode->connect(*gnode,"");
	  }
	
	}
	  graph->write (std::cerr);
  }

  void createLoop (const std::string& name, const MiniMC::Model::Analysis::LoopInfo& linfo, const MiniMC::Model::CFG& cfg) {
	auto graph = MiniMC::Support::CreateGraph<MiniMC::Support::GraphType::DOT> (name);
	for (auto& l : linfo) { 
	  auto hnode = graph->getNode (l->getHeader() -> getName ());
	  auto eit = l->internal_begin ();
	  for (; eit != l->internal_end (); ++eit) {
		auto fnode = graph->getNode ((*eit)->getFrom ()->getName());
		auto tnode = graph->getNode ((*eit)->getTo ()->getName());
		fnode->connect(*tnode,"");
	  }
	  eit = l->exiting_begin ();
	  for (; eit != l->exiting_end (); ++eit) {
		auto fnode = graph->getNode ((*eit)->getFrom ()->getName());
		auto tnode = graph->getNode ((*eit)->getTo ()->getName());
		fnode->connect(*tnode,"");
	  }

	  eit = l->back_begin();
	  for (; eit != l->back_end (); ++eit) {
		auto fnode = graph->getNode ((*eit)->getFrom ()->getName());
		auto tnode = graph->getNode ((*eit)->getTo ()->getName());
		fnode->connect(*tnode,"");
	  }
	  
	}
	
	graph->write (std::cout);
  }

  
  
template<class CPADef>
void runAlgorithm (MiniMC::Model::Program& prgm, MiniMC::Algorithms::SetupOptions sopt) {
  using algorithm = MiniMC::Algorithms::PrintCPA<CPADef>;
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm<algorithm> (seq,sopt);

  for (auto& func : prgm.getEntryPoints ()) {
	auto dominatorinfo = MiniMC::Model::Analysis::calculateDominators (func->getCFG ());
	createDomTree (func->getName(),dominatorinfo,*func->getCFG());	
  }

  for (auto& func : prgm.getEntryPoints ()) {
	
	auto loopinfo = MiniMC::Model::Analysis::createLoopInfo (func->getCFG ());
	createLoop (func->getName()+"Loop",loopinfo,*func->getCFG());	
  }
  
  
  
  //MiniMC::Savers::OptionsSave<MiniMC::Savers::Type::JSON>::Opt saveOpt {.writeTo = &std::cout};
  //MiniMC::Savers::saveModel<MiniMC::Savers::Type::JSON> (prgm.shared_from_this (),saveOpt);
  
}
  
enum class CPAUsage {
					 Location,
					 LocationExplicit,
					 CVC4PathFormula
};

}

int dom_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, MiniMC::Algorithms::SetupOptions& sopt) {
  CPAUsage CPA = CPAUsage::Location;
  po::options_description desc("Conversion Options");
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
     )
    ("expandnondet",po::bool_switch (&sopt.expandNonDet),"Expand all non-deterministic values")
    ("splitcmps",po::bool_switch (&sopt.splitCMPS),"Split control-flow at comparisons")
	("inlinefunctions",po::value<std::size_t> (&sopt.inlinefunctions),"Inline function calls")
	("unrollloops",po::value<std::size_t> (&sopt.unrollLoops),"Unroll Loops")
	
	;
  
  
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
  switch (CPA) {
  case CPAUsage::CVC4PathFormula:
	runAlgorithm<CVC4Path> (*prgm,sopt);
	break;
  case CPAUsage::LocationExplicit:
    runAlgorithm<LocExpliStack> (*prgm,sopt);
    break;
  case CPAUsage::Location:
  default:
	runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt);
    break;
  }

  return static_cast<int> (0);
}


static CommandRegistrar dom_reg ("dominator",dom_main,"Output dominator graph");
