#ifndef __LOCATION__
#define __LOCATION__

#include "support/types.hpp"
#include "model/source.hpp"
#include "hash/hashing.hpp"

#include <cassert>
#include <gsl/pointers>


namespace MiniMC {
  namespace Model {

    class Edge;
    using Edge_ptr = std::shared_ptr<Edge>;
    using Edge_wptr = std::weak_ptr<Edge>;

    template <class Obj, class BaseIterator>
    class SmartIterator {
    public:
      SmartIterator(BaseIterator iter) : iter(iter) {}
      bool operator==(const SmartIterator<Obj, BaseIterator>& oth) const {
        return oth.iter == iter;
      }

      bool operator!=(const SmartIterator<Obj, BaseIterator>& oth) const {
        return oth.iter != iter;
      }

      Obj operator->() {
        assert((*iter).lock());
        return (*iter).lock();
      }

      Obj operator*() {
        assert((*iter).lock());
        return (*iter).lock();
      }

      void operator++() {
        ++iter;
      }

    private:
      BaseIterator iter;
    };

    class CFA;
    using CFA_ptr = std::shared_ptr<CFA>;
    using CFA_wptr = std::weak_ptr<CFA>;
    /** 
     * Location in a functions CFG
     * Locations can be assigned different attributes which can affect exploration algorithms, or the modification algorihtms on the CFG. 
     */
    class Location : public std::enable_shared_from_this<Location> {
    public:
      friend class CFA;
      friend class Edge;

      using edge_iterator = SmartIterator<Edge_ptr, std::vector<Edge_wptr>::iterator>;

      Location(const LocationInfo& n, MiniMC::offset_t id, CFA* cfg) : info(n), id(id), cfg(cfg) {}

      void addEdge(gsl::not_null<Edge_ptr> e) { edges.push_back(e.get()); }
      void addIncomingEdge(gsl::not_null<Edge_ptr> e) { incomingEdges.push_back(e.get()); }
      /** 
       *
       * @return begin iterator for outgoing edges
       */
      edge_iterator ebegin() { return SmartIterator<Edge_ptr, std::vector<Edge_wptr>::iterator>(edges.begin()); }

      /** 
       *
       * @return end iterator for outgoing edges
       */
      edge_iterator eend() { return SmartIterator<Edge_ptr, std::vector<Edge_wptr>::iterator>(edges.end()); }

      /** 
       *
       * @return begin iterator for incoming edges
       */
      edge_iterator iebegin() { return SmartIterator<Edge_ptr, std::vector<Edge_wptr>::iterator>(incomingEdges.begin()); }

      /** 
       *
       * @return end iterator for incoming edges
       */
      edge_iterator ieend() { return SmartIterator<Edge_ptr, std::vector<Edge_wptr>::iterator>(incomingEdges.end()); }

      /** 
       * Check if this location has outoing edges
       *
       * @return true if it has outgoing edges, false if not
       */
      bool hasOutgoingEdge() const {
        return edges.size();
      }

      auto nbOutgoingEdges() const {
        return edges.size();
      }

      MiniMC::Hash::hash_t hash () const {
	return getID ();
      }
      
      const LocationInfo& getInfo() const { return info; }
      LocationInfo& getInfo() { return info; }

      /** 
       * Count the number of incoming edges
       *
       *
       * @return number of incoming edges
       */
      auto nbIncomingEdges() const {
        return incomingEdges.size();
      }

      MiniMC::offset_t getID() const { return id; }

      bool isOutgoing(const MiniMC::Model::Edge_ptr& e) {
        auto it = std::find_if(edges.begin(), edges.end(),
                               [&e](const Edge_wptr& ptr1) {
                                 return ptr1.lock() == e;
                               });
        return it != edges.end();
      }

      bool isIncoming(const MiniMC::Model::Edge_ptr& e) {
        auto it = std::find_if(incomingEdges.begin(), incomingEdges.end(),
                               [&e](const Edge_wptr& ptr1) {
                                 return ptr1.lock() == e;
                               });
        return it != incomingEdges.end();
      }

      CFA* getCFG() const {
        return cfg;
      }

    protected:
      /** 
       * Search for an edge \p e. If found delete it from the outgoing edges.
       * Notice \p e is searched for using pointer equality.  
       *
       * @param e The edge to search for
       */
      void removeEdge(const Edge_ptr e) {
        auto it = std::find_if(edges.begin(), edges.end(),
                               [&e](const Edge_wptr& ptr1) {
                                 return ptr1.lock() == e;
                               });

        assert(it != edges.end());
        edges.erase(it);
      }

      /** 
       * Search for an edge \p e. If found delete it from the incoming  edges.
       * Notice \p e is searched for using pointer equality.  
       *
       * @param e The edge to search for
       */
      void removeIncomingEdge(const Edge_ptr e) {
        auto it = std::find_if(incomingEdges.begin(), incomingEdges.end(),
                               [&e](const Edge_wptr& ptr1) {
                                 return ptr1.lock() == e;
                               });
        assert(it != incomingEdges.end());
        incomingEdges.erase(it);
      }

    private:
      std::vector<Edge_wptr> edges;
      std::vector<Edge_wptr> incomingEdges;
      LocationInfo info;
      MiniMC::offset_t id;
      CFA* cfg;
    };

    using Location_ptr = std::shared_ptr<Location>;
    using Location_wptr = std::weak_ptr<Location>;
    
  } // namespace Model
} // namespace MiniMC


#endif
