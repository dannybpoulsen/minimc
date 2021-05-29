/**
 * @file   algorithm.hpp
 * @date   Mon Apr 20 15:46:57 2020
 * 
 * @brief  
 * 
 * 
 */
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
#include "model/modifications/replacenondetuniform.hpp"
#include "model/modifications/markinglooplocations.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/replacesub.hpp"
#include "model/modifications/splitcmps.hpp"
#include "model/modifications/constantfolding.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/removealloca.hpp"
#include "model/modifications/removephi.hpp"


#include "model/analysis/manager.hpp"

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
	
	
	/**
	   Algorithm 
	*/
    class Algorithm {
    public:
      Algorithm ()  {}
      Algorithm (const Algorithm& ) = default;
	  virtual Result run (const MiniMC::Model::Program& prgm) = 0;
	  
	};

	
	
	/**  
	 *	Possible State space reductions. MiniMC can reduce the stored
	 *  state space by storing states that are in locations marked as
	 *  important. 	 
	 */  
	enum class SpaceReduction  {
	  None, /**< Mark all states as important*/
	  Conservative, /**< Mark states that are part of a loop*/ 
	  Extreme /**< Mark no states - effectively meaning we store absolutely no states. */
	};
	
	struct SetupOptions {
	  SpaceReduction reduction;
	  bool isConcurrent = false;
	  bool expandNonDet = false;
	  bool replaceNonDetUniform = false;
	  bool simplifyCFG = false;
	  bool replaceSub = false;
	  bool splitCMPS = false;
	  bool foldConstants = false;
	  bool convergencePoints = false;
	  bool removeAllocs = false;
	  bool replacememnodet = false;
	  bool removephi = false;
	  std::size_t inlinefunctions = 0;
	  std::size_t unrollLoops = 0;
	  
	};
	
    template<class W, class ...Args>
    using BaseAWrapper = MiniMC::Support::SequenceWrapper<MiniMC::Model::Program,W,Args...>;
	
    template<class W, class ...Args>
    using AWrapper = BaseAWrapper<W,Args...>;

	/** 
	 * Add the typechecking, structural checks and modifications needed by \tparam algorithm to the MiniMC::Support::Sequencer<MiniMC::Model::Program> \p seq
	 */
    inline void  setupForAlgorithm (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, const SetupOptions& options) {
	  seq.template add<MiniMC::Model::Modifications::InsertBoolCasts> ();  
	  seq.template add<MiniMC::Model::Checkers::TypeChecker> ();
	  seq.template add<MiniMC::Model::Checkers::StructureChecker> ();
	  if (options.removephi) {
		seq.template add<MiniMC::Model::Modifications::RemovePhi> ();	
	  }
	  if (options.replacememnodet) {
		seq.template add<MiniMC::Model::Modifications::RemoveMemNondet> ();
	  }
	  seq.template add<MiniMC::Model::Modifications::SplitAsserts> ();
	  
	  if (options.inlinefunctions) {
	    seq.template add<MiniMC::Model::Modifications::InlineFunctions,std::size_t> (options.inlinefunctions); 
	  }
	  
	  if (options.unrollLoops) 
		seq.template add<MiniMC::Model::Modifications::UnrollLoops> ( options.unrollLoops);
	  
	  
	  if (options.foldConstants) {
	    seq.template add<MiniMC::Model::Modifications::FoldConstants> ();
	  }
	  if (options.splitCMPS) {
	    seq.template add<MiniMC::Model::Modifications::EnsureEdgesOnlyHasOneCompar> ();
	    seq.template add<MiniMC::Model::Modifications::SplitCompares> ();
	  }
	  
	  seq.template add<MiniMC::Model::Modifications::KillUnneededBranching> ();
	  seq.template add<MiniMC::Model::Modifications::LowerGuards> ();  
	  seq.template add<MiniMC::Model::Modifications::RemoveUnneededCallPlaceAnnotations> ();
	  if (options.replaceSub) {
	    seq.template add<MiniMC::Model::Modifications::ReplaceSub> ();  
	  }
	  if (options.expandNonDet) {
	    seq.template add<MiniMC::Model::Modifications::ExpandNondet> ();  
	  }
	  if (options.replaceNonDetUniform) {
	    seq.template add<MiniMC::Model::Modifications::ReplaceNonDetUniform> ();  
	  }
	  seq.template add<MiniMC::Model::Modifications::RemoveRetEntryPoints> ();
	  if (options.simplifyCFG)
	    seq.template add<MiniMC::Model::Modifications::SimplifyCFG> (); 
	  if (options.isConcurrent) {
	    seq.template add<MiniMC::Model::Modifications::EnsureEdgesOnlyHasOneMemAccess> ();  
	  }
	  if (options.removeAllocs)  
		seq.template add<MiniMC::Model::Modifications::RemoveAllocas> (); 
	  
	  if (options.convergencePoints) {
		seq.template add<MiniMC::Model::Modifications::ConvergencePointAnnotator> ();  	  
	  }
	  
	  seq.template add<MiniMC::Model::Checkers::TypeChecker> ();
	  seq.template add<MiniMC::Model::Checkers::StructureChecker> ();  
	  
	  if  (options.reduction == SpaceReduction::Conservative) {
	    seq.template add<MiniMC::Model::Modifications::MarkLoopStates> (); 
	  }
	  else if (options.reduction == SpaceReduction::None) {
	    seq.template add<MiniMC::Model::Modifications::MarkAllStates> ();
	  }
	  
	}

	/** First run the modications of \p seq on \p prgm and then run \p algo.
		\returns Result::Error if running \p seq failed, other the result of algo.run (prgm)
	*/ 
	/*template<class Seq, class Algo>
	auto runSetup (Seq& seq, Algo& algo, MiniMC::Model::Program& prgm) {
	  if (seq.run (prgm)) {
	    return algo.run (prgm);
	  }
	  return Result::Error;
	  }*/
	
  }
}

#endif
