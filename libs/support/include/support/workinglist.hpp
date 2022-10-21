#ifndef _WORKING_LIST__
#define _WORKING_LIST__

#include <functional>
#include <vector>

namespace MiniMC {
  namespace Support {
    /**
	 * Temporary working list object, which is useful when you have to
	 * iterate over a collection that may change during the iteration.  
	 *
	 */
    template <class Element, class Container = std::vector<Element>>
    class WorkingList {
    public:
      auto inserter() { return std::back_inserter(container); }
      auto begin() { return container.begin(); }
      auto end() { return container.end(); }
      auto size() const { return container.size(); }
      bool empty() const { return !size(); }
      auto pop() {
        Element elem = container.back();
        container.pop_back();
        return elem;
      }

    private:
      Container container;
    };

  } // namespace Support
} // namespace MiniMC

#endif
