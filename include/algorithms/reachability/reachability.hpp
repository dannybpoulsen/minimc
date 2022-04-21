
#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"


namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {
      using GoalFunction = std::function<bool(const MiniMC::CPA::State_ptr&)>;
      
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
	
	Verdict search (const MiniMC::CPA::State_ptr&, GoalFunction);
	Observable<Progress>& getPWProgresMeasure ()  {return progress_indicator;}
	
      private:
	Observable<Progress> progress_indicator;
	MiniMC::CPA::Transferer_ptr transfer;
	MiniMC::CPA::Joiner_ptr joiner;
	
      };
    }
  }
}
