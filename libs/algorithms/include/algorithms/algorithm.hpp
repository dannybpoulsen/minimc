#ifndef _ALGORITHMS__
#define _ALGORITHMS__

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
		       Success,
		       Error,
		       Stopped
    };

    class StopCriterion {
    public:
      virtual bool shouldStop () {return false;}
    };

    class Algorithm {
    public:
      Algorithm (MiniMC::Support::Messager& m) : messager(&m) {}
      Algorithm (const Algorithm& ) = default;
      void setStopper (StopCriterion* stopper) {this->stopper = stopper;}
      virtual Result run (const MiniMC::Model::Program&) {
	messager->message ("Starting dummy algorithm");
	messager->message ("Finisheddummy algorithm");
	return Result::Success;
      }
      
      static void presetups (MiniMC::Support::Sequencer<MiniMC::Model::Program>&, MiniMC::Support::Messager&) {}
      
    protected:
      bool stopEarly () const {
	if (stopper) {
	  return stopper->shouldStop ();
	}
	else {
	  return false;
	}
      }
      MiniMC::Support::Messager& getMessager () const {return *messager;}
    private:
      StopCriterion* stopper = nullptr;
      MiniMC::Support::Messager* messager;
    };

	enum class SpaceReduction {
							   None,
							   Conservative,
							   Extreme
	};

    template<class W, class ...Args>
    using BaseAWrapper = MiniMC::Support::SequenceWrapper<MiniMC::Model::Program,W,Args...>;

    template<class W, class ...Args>
    using AWrapper = BaseAWrapper<W,MiniMC::Support::Messager&,Args...>;

    template<class algorithm,class ...Args>
    void  setupForAlgorithm (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess, SpaceReduction reduct, Args... args) {
		seq.template add<MiniMC::Model::Modifications::InsertBoolCasts> ();  
		seq.template add<MiniMC::Model::Checkers::TypeChecker, MiniMC::Support::Messager&> (mess);
		seq.template add<MiniMC::Model::Checkers::StructureChecker, MiniMC::Support::Messager&> (mess);  
		seq.template add<MiniMC::Model::Modifications::SplitAsserts> ();  
		seq.template add<MiniMC::Model::Modifications::LowerGuards> ();  
		seq.template add<MiniMC::Model::Modifications::RemoveUnneededCallPlaceAnnotations> (); 
		seq.template add<MiniMC::Model::Modifications::SimplifyCFG> (); 
		algorithm::presetups (seq,mess);
		if  (reduct == SpaceReduction::Conservative) {
		  seq.template add<MiniMC::Model::Modifications::MarkLoopStates> (); 
		}
		else if (reduct == SpaceReduction::None) {
		  seq.template add<MiniMC::Model::Modifications::MarkAllStates> ();
		}
		seq.template add<MiniMC::Algorithms::AWrapper<algorithm,Args...>, MiniMC::Support::Messager&,Args...> (mess, args...);
		
	}
  }
}

#endif
