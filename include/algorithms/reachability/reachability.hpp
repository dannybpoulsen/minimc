
#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"

#include <iosfwd>

namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {
      
      enum class StateStatus {
	Keep,
	Discard
      };
      using GoalFunction = std::function<bool(const MiniMC::CPA::State_ptr&)>;
      using FilterFunction = std::function<StateStatus(const MiniMC::CPA::State_ptr&)>;

      StateStatus DefaultFilter (const MiniMC::CPA::State_ptr&);; 
      
      enum class Verdict {
	Found,
	NotFound
      };
      
      class Reachability {
      public:
	struct Progress {
	  std::size_t passed{0};
	  std::size_t waiting{0};
	};
	
	Reachability (MiniMC::CPA::Transferer_ptr transfer,
		      MiniMC::CPA::Joiner_ptr joiner) : transfer(transfer), joiner(joiner)  {}
	
	Verdict search (const MiniMC::CPA::State_ptr&,
			GoalFunction,
			FilterFunction = DefaultFilter
			);


	MiniMC::CPA::State_ptr foundState () const {return found;}
	
	Observable<Progress>& getPWProgresMeasure ()  {return progress_indicator;}
	
      private:
	Observable<Progress> progress_indicator;
	MiniMC::CPA::Transferer_ptr transfer;
	MiniMC::CPA::Joiner_ptr joiner;
	MiniMC::CPA::State_ptr found;
      };


      inline std::ostream& operator<< (std::ostream& os, const Reachability::Progress& prgs) {
	return os << "Passed: " << prgs.passed << " Waiting:" << prgs.waiting;
	
      }
      
    }
  }
}
