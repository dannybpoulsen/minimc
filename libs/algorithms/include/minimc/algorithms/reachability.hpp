
#include "minimc/cpa/interface.hpp"
#include "minimc/support/feedback.hpp"

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


      class Result {
      public:
	Result (MiniMC::CPA::AnalysisState&& state,
		std::size_t exploredStates) : _verdict(Verdict::Found),
					      _state(std::move(state)),
					      _exploredStates(exploredStates) {}

	Result (std::size_t exploredStates) : _verdict(Verdict::NotFound),
					      _exploredStates(exploredStates) {}

	auto verdict () const {return _verdict;}
	auto foundState () const {return _state;}
	auto exploredStates () const {return _exploredStates;}
	
	
      private:
	
	Verdict _verdict;
	MiniMC::CPA::AnalysisState _state;
	std::size_t _exploredStates;
      };
      

      
      class Reachability {
      public:
	Reachability (MiniMC::CPA::AnalysisTransfer transfer);
	~Reachability ();
	[[nodiscard]] Result search (MiniMC::Support::Messager& mess,
			const MiniMC::CPA::AnalysisState&,
				     GoalFunction,
				     FilterFunction = DefaultFilter
				     );
	
	
	void setSearchStrategy (SearchStrategy);
	
      private:
	struct Internal;
	std::unique_ptr<Internal> _internal;
      };
      
      
    }
  }
}
