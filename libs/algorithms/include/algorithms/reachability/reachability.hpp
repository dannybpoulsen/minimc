
#include "cpa/interface.hpp"
#include "support/feedback.hpp"

#include <iosfwd>
#include <memory>

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


      enum class SearchStrategy {
	DFS,
	BFS
      };
      
      class Reachability {
      public:
	Reachability (MiniMC::CPA::AnalysisTransfer transfer);
	~Reachability ();
	Verdict search (MiniMC::Support::Messager& mess,
			const MiniMC::CPA::AnalysisState&,
			GoalFunction,
			FilterFunction = DefaultFilter
			);
	
	
	MiniMC::CPA::AnalysisState foundState () const;
	std::size_t getNumberExploredStates () const;

	void setSearchStrategy (SearchStrategy);
	
      private:
	struct Internal;
	std::unique_ptr<Internal> _internal;
};
      
      
    }
  }
}
