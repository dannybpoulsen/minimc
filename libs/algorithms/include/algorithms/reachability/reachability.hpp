
#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"
#include "algorithms/reachability/verdict.hpp"

#include <iosfwd>

namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {
      
      enum class StateStatus {
	Keep,
	Discard
      };
      using GoalFunction = std::function<bool(const MiniMC::CPA::AnalysisState&)>;
      using FilterFunction = std::function<StateStatus(const MiniMC::CPA::AnalysisState&)>;

      StateStatus DefaultFilter (const MiniMC::CPA::AnalysisState&);; 
      

      class Reachability {
      public:
	struct Progress {
	  std::size_t passed{0};
	  std::size_t waiting{0};
	};
	
	Reachability (MiniMC::CPA::AnalysisTransfer transfer) : transfer(transfer)  {}
	
	Verdict::Details search (const MiniMC::CPA::AnalysisState&,
			GoalFunction,
			FilterFunction = DefaultFilter
			);


	Observable<Progress>& getPWProgresMeasure ()  {return progress_indicator;}

      private:
	Observable<Progress> progress_indicator;
	MiniMC::CPA::AnalysisTransfer transfer;
	MiniMC::CPA::Joiner_ptr joiner;
      };
      

      inline std::ostream& operator<< (std::ostream& os, const Reachability::Progress& prgs) {
	return os << "Passed: " << prgs.passed << " Waiting:" << prgs.waiting;
	
      }
      
    }
  }
}
