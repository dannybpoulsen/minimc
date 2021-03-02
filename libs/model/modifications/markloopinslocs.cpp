#include <unordered_map>
#include <stack>

#include "support/workinglist.hpp"
#include "model/modifications/markinglooplocations.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
	  enum class Colour {
						 Unvisited,
						 OnStack,
						 Done
	  };

	  struct InnerRun {
	  public:
		void mainSearch (MiniMC::Model::CFG& cfg) {
		  MiniMC::Support::WorkingList<MiniMC::Model::Location*> wlist;
		  auto inserter = wlist.inserter ();
		  for (auto& l : cfg.getLocations ()) {
			colours.insert (std::make_pair (l.get(),Colour::Unvisited));
			inserter = l.get();
		  }

		  for (auto& loc : wlist) {
			if (colours.at(loc) == Colour::Unvisited) {
			  dfsRecursive (loc);
			}
		  }

		  
		}
		
		void dfsRecursive (MiniMC::Model::Location* l) {
		  struct Actions {
			Colour col;
			MiniMC::Model::Location* loc;
		  };
		  std::stack<Actions> stack;
		  auto addToSearch  = [&](MiniMC::Model::Location* loc) {
								
								stack.push ( { .col = Colour::Done, .loc = loc});
								stack.push ( {.col = Colour::OnStack, .loc = loc});

							  };
		  addToSearch (l);
		  while (!stack.empty()) {
			
			auto next = stack.top ();
			stack.pop ();
			colours[next.loc] = next.col;
			if (next.col == Colour::OnStack) {
			  auto it = next.loc->ebegin();
			  auto end = next.loc->eend ();
			  for (; it!=end; ++it) {
				auto to = it->getTo().get().get();
				if (colours.at(to) == Colour::OnStack) {
				  //We have a loop
				  to->getInfo().template set <MiniMC::Model::Attributes::NeededStore> ();
				  
				}
				else if (colours.at(to) == Colour::Unvisited) {
				  
				  addToSearch (to);
				}
			  }
			}
			
		  }
		}
		
	  private:
		std::unordered_map<MiniMC::Model::Location*, Colour> colours;
		
	  };
	  
	  bool MarkLoopStates::run (MiniMC::Model::Program&  prgm) {
		for (auto& F : prgm.getFunctions ()) {
		  InnerRun runner;
		  runner.mainSearch (*F->getCFG ());
		  
		}
		return true;
	  }
	}
  }
}
