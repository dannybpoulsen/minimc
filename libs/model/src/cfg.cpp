#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "support/workinglist.hpp"


namespace MiniMC {
  namespace Model {
    void CFA::deleteLocation(const Location_ptr& location) {
        MiniMC::Support::WorkingList<MiniMC::Model::Edge*> wlist;
        auto insert = wlist.inserter();
        std::for_each(location->ebegin(), location->eend(), [&](const auto& e) { insert = e; });
        std::for_each(location->iebegin(), location->ieend(), [&](const auto& e) { insert = e; });
	
        std::for_each(wlist.begin(), wlist.end(), [&](const auto& e) { this->deleteEdge(e); });
	
        auto it = std::find(locations.begin(), locations.end(), location);
        if (it != locations.end()) {
          locations.erase(it);
        }
      }
    
  } // namespace Model
} // namespace MiniMC
