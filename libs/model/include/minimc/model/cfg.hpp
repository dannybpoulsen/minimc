
/**
 * @file   cfg.hpp
 * @date   Sun Apr 19 20:16:17 2020
 * 
 * 
 */
#ifndef _CFG__
#define _CFG__

#include <algorithm>
#include <functional>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "minimc/model/edge.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/location.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/model/heaplayout.hpp"
#include "minimc/model/symbol.hpp"
#include "minimc/host/types.hpp"

namespace MiniMC {
  namespace Model {

    class Function;
    using Function_ptr = std::shared_ptr<Function>;
    
    /**
	 *
	 * Representation of an Control Flow Automaton. 
	 * The CFA is responsible for creating( and deleting) edges and
	 * locations of a function. It will also make sure that  the
	 * incoming/outgoing edges of locations are properly update when
	 * deleting edges. This 
	 *
	 */
    class CFA  {
    public:
      CFA () {}
      CFA (const CFA& ) = delete;
      CFA (CFA&& cfa) = default;
      Location_ptr makeLocation(MiniMC::Model::Symbol symbol, const LocationInfo& info) {
        locations.push_back(std::make_shared<Location>(symbol,info, locations.size()));
	symbol.setUserData (locations.back());
	return locations.back();
      }

      CFA& operator=(CFA&&) = default;
      
      /**
       * Make a new edge
       *
       * @param from source of the edge

* @param to target of the edge
       *
       * @return 
       */
      Edge_ptr makeEdge(Location_ptr from, Location_ptr to, InstructionStream&& istream,bool isPhi = false  ) {
	assert(std::find(locations.begin(),locations.end(),to) != locations.end());
	assert(std::find(locations.begin(),locations.end(),from) != locations.end());
	edges.push_back(std::make_shared<Edge>(from, to,std::move(istream),isPhi));
        to->addIncomingEdge(edges.back().get());
        from->addEdge(edges.back().get());
        return edges.back();
      }

      Location_ptr getInitialLocation() const {
        assert(initial);
        return initial;
      }

      void setInitial(Location_ptr loc) {
        initial = loc;
      }

      /** 
       * Delete \p edge from this CFG. Update also the
       * incoming/outgoing edges of the target/source of \p edge. 
       *
       * @param edge The edge to delete
       */
      void deleteEdge(const Edge* edge) {
        edge->getFrom()->removeEdge(edge);
        edge->getTo()->removeIncomingEdge(edge);
	
        auto it = std::find_if(edges.begin(), edges.end(), [edge](auto& e) {return e.get() == edge;});
        if (it != edges.end()) {
          edges.erase(it);
        }
      }
      
      void deleteLocation(const Location_ptr& location);

      auto& getLocations() const { return locations; }
      auto& getLocations() { return locations; }
      auto& getEdges() const { return edges; }
      
      

    private:
      std::vector<Location_ptr> locations;
      std::vector<Edge_ptr> edges;
      Location_ptr initial = nullptr;
    };

   
  } // namespace Model
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::Model::Location> {
    std::size_t operator()(const MiniMC::Model::Location& loc) { return reinterpret_cast<size_t>(&loc); }
  };
} // namespace std

#endif
