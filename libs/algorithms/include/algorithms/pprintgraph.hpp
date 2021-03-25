#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__
#include <gsl/pointers>
#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/psuccessorgen.hpp"
#include "cpa/location.hpp"
#include "cpa/concrete.hpp"
#include "cpa/compound.hpp"


namespace MiniMC {
  namespace Algorithms {
    template<class SMC>
    class ProbaChecker : public MiniMC::Algorithms::Algorithm {
    public:
	  struct Options  {
		MiniMC::Support::Messager* messager;
		std::size_t len;
		typename SMC::Options smcoptions;
	  };
	
      ProbaChecker (const Options& opt) : messager(*opt.messager), smc(opt.smcoptions),length(opt.len) {}
      using CPA = MiniMC::CPA::Compounds::CPADef<0,
						       MiniMC::CPA::Location::CPADef,
						       MiniMC::CPA::Concrete::CPADef
						       >;
      virtual Result run (const MiniMC::Model::Program& prgm) {
		if (!CPA::PreValidate::validate (prgm,messager)) {
		  return Result::Error;
		}
		auto initstate = CPA::Query::makeInitialState (prgm);
		try {
		  while (smc.continueSampling()) {
			generateTrace (initstate);
		  }
		}
		catch(MiniMC::Support::VerificationException& exc) {
		  messager.error (exc.what());
		}
		
		messager.message (MiniMC::Support::Localiser {"Probability of (<> AssertViolated) estimated to: [%1%, %2%]"}.format (smc.lProbability (),smc.hProbability ()));
		return Result::Success;
      }
	  
      void generateTrace (MiniMC::CPA::State_ptr state ) {
		auto cur = state;
		for (size_t i = 0; i< length; i++) {
		  auto it = (MiniMC::Algorithms::Proba::Generator<CPA::Query,CPA::Transfer> (cur)).begin();
		  cur = it->state;
		  if (!cur)
			break;
		  auto loc = cur->getLocation (it->proc);
		  if (loc->getInfo().template is<MiniMC::Model::Attributes::AssertViolated> ()) {
			smc.sample (MiniMC::Support::Statistical::Result::Satis);
			return;
		  }
		}
		smc.sample (MiniMC::Support::Statistical::Result::NSatis);
      }
	  

	private:
	  MiniMC::Support::Messager& messager;
      SMC smc;
      std::size_t length;
	  
    };
      
      
    
  }
}

#endif
