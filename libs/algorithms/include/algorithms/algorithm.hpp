#ifndef _ALGORITHMS__
#define _ALGORITHMS__

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/exceptions.hpp"

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
      void setStopper (StopCriterion* stopper) {this->stopper = stopper;}
      virtual Result run (const MiniMC::Model::Program&) {
	messager->message ("Starting dummy algorithm");
	messager->message ("Finisheddummy algorithm");
	return Result::Success;
      }
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
    
  }
}

#endif
