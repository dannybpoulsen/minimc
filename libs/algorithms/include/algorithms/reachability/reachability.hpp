
#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"
#include "support/feedback.hpp"

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
      
      enum class Verdict {
	Found,
	NotFound
      };

      struct Progress  {
	Progress (std::size_t passed, std::size_t waiting) : passed(passed),waiting(waiting) {}
	std::size_t passed{0};
	std::size_t waiting{0};
      };

      class Reachability {
      public:
	
	
	Reachability (MiniMC::CPA::AnalysisTransfer transfer) : transfer(transfer)  {}
	
	Verdict search (MiniMC::Support::Messager& mess,
			const MiniMC::CPA::AnalysisState&,
			GoalFunction,
			FilterFunction = DefaultFilter
			);

	
	//Observable<Progress>& getPWProgresMeasure ()  {return progress_indicator;}
	MiniMC::CPA::AnalysisState foundState () const {return found;}
	auto getNumberExploredStates () const {return nbExploredStates;};
      private:
	MiniMC::CPA::AnalysisTransfer transfer;
	MiniMC::CPA::AnalysisState found;
	std::size_t nbExploredStates;
      };
      
      
    }
  }
}
