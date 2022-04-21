
#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"


namespace MiniMC {
  namespace Algorithms {
    using GoalFunction = std::function<bool(const MiniMC::CPA::State_ptr&)>;

    enum class Verdict {
      Found,
      NotFound
    };
    
    class Reachability {
    public:
      struct Progress {
	Progress (std::size_t passed, std::size_t waiting) : passed(passed),waiting(waiting) {}
	std::size_t passed;
	std::size_t waiting;
      };

      Reachability (MiniMC::CPA::Transferer_ptr transfer,
		    MiniMC::CPA::Joiner_ptr joiner)  {}

      Verdict search (MiniMC::CPA::State_ptr);
      Observable<Progress>& getPWProgresMeasure ()  {return progress_indicator;}
      
    private:
      Observable<Progress> progress_indicator;
      MiniMC::CPA::Transferer_ptr transfer;
      MiniMC::CPA::JOiner_ptr joiner;
      
      };
    
  }
}
