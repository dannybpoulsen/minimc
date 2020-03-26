#ifndef _ALGORITHMS__
#define _ALGORITHMS__

#include <gsl/pointers>

#include "model/cfg.hpp"
#include "model/modifications/replacememnondet.hpp"
#include "model/modifications/insertboolcasts.hpp"
#include "model/modifications/splitasserts.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/lower_guards.hpp"
#include "model/modifications/simplify_cfg.hpp"
#include "model/modifications/markinglooplocations.hpp"

#include "model/checkers/typechecker.hpp"
#include "model/checkers/structure.hpp"
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/sequencer.hpp"


namespace MiniMC {
  namespace Algorithms {
    enum class Result {
					   Success = 0,
					   Error = 1
    };


	
    class Algorithm {
    public:
      Algorithm ()  {}
      Algorithm (const Algorithm& ) = default;
      virtual Result run (const MiniMC::Model::Program&) {
		
		return Result::Success;
      }
      
      static void presetups (MiniMC::Support::Sequencer<MiniMC::Model::Program>&, MiniMC::Support::Messager&) {}
    };

	enum class SpaceReduction {
							   None,
							   Conservative,
							   Extreme
	};

	struct SetupOptions {
	  gsl::not_null<MiniMC::Support::Messager*> messager;
	  SpaceReduction reduction;
	};
	
    template<class W, class ...Args>
    using BaseAWrapper = MiniMC::Support::SequenceWrapper<MiniMC::Model::Program,W,Args...>;
	
    template<class W, class ...Args>
    using AWrapper = BaseAWrapper<W,Args...>;

    template<class algorithm>
    void  setupForAlgorithm (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, const SetupOptions& options) {
		seq.template add<MiniMC::Model::Modifications::InsertBoolCasts> ();  
		seq.template add<MiniMC::Model::Checkers::TypeChecker, MiniMC::Support::Messager&> (*options.messager);
		seq.template add<MiniMC::Model::Checkers::StructureChecker, MiniMC::Support::Messager&> (*options.messager);  
		seq.template add<MiniMC::Model::Modifications::SplitAsserts> ();  
		seq.template add<MiniMC::Model::Modifications::LowerGuards> ();  
		seq.template add<MiniMC::Model::Modifications::RemoveUnneededCallPlaceAnnotations> (); 
		seq.template add<MiniMC::Model::Modifications::SimplifyCFG> (); 
		
		algorithm::presetups (seq,*options.messager);

		if  (options.reduction == SpaceReduction::Conservative) {
		  seq.template add<MiniMC::Model::Modifications::MarkLoopStates> (); 
		}
		else if (options.reduction == SpaceReduction::None) {
		  seq.template add<MiniMC::Model::Modifications::MarkAllStates> ();
		}
		
	}

	template<class Seq, class Algo>
	auto runSetup (Seq& seq, Algo& algo, MiniMC::Model::Program& prgm) {
	  if (seq.run (prgm)) {
		return algo.run (prgm);
	  }
	  return Result::Error;
	}
	
  }
}

#endif
