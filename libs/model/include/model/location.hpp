#ifndef __LOCATION__
#define __LOCATION__


namespace MiniMC {
  namespace Model {    

	

	class Edge;    
	using Edge_ptr = std::shared_ptr<Edge>;
	using Edge_wptr = std::weak_ptr<Edge>;
	
	template<class Obj, class BaseIterator>
	class SmartIterator {
	public:
	  SmartIterator (BaseIterator iter) : iter(iter) {}
	  bool operator== (const SmartIterator<Obj,BaseIterator>& oth) const  {
		return oth.iter == iter;
	  }

	  bool operator!= (const SmartIterator<Obj,BaseIterator>& oth) const {
		return oth.iter != iter;
	  }

	  Obj operator-> () {
		return (*iter).lock();
	  }

	  Obj operator* () {
		return (*iter).lock();
	  }

	  void operator++ () {
		++iter;
	  }
	  
	private:
	  BaseIterator iter;
	};

	/** 
	 * Location in a functions CFG
	 * Locations can be assigned different attributes which can affect exploration algorithms, or the modification algorihtms on the CFG. 
	 */
    class Location : public std::enable_shared_from_this<Location>{
    public:
	  friend class CFG;
	  friend class Edge;
      using AttrType = char;

	  /**
	   * The possible attributes that can be assigned locations
	   *
	   */
      enum class Attributes  : AttrType {
		AssertViolated = 1, /**< Indicates an assert was violated */
		NeededStore = 2, /**< Indicates this location is part of loop, and must be stored for guaranteeing termination*/
		CallPlace = 4, /**< Indicates a call takes place on an edge leaving this location */
		AssumptionPlace = 8, /**< Indicates an assumption is made on an edge leaving this location  */
		ConvergencePoint = 16
	  };
      
      using edge_iterator = SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator>;
      
      Location (const std::string& n, MiniMC::offset_t id) : name(n),id(id) {}
      
      void addEdge (gsl::not_null<Edge_ptr> e) {edges.push_back(e.get());}
      void addIncomingEdge (gsl::not_null<Edge_ptr> e) {incomingEdges.push_back(e.get());}
      /** 
	   *
	   * @return begin iterator for outgoing edges
	   */
      edge_iterator ebegin () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (edges.begin());}

	  /** 
	   *
	   * @return end iterator for outgoing edges
	   */
	  edge_iterator eend () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (edges.end());}

	  /** 
	   *
	   * @return begin iterator for incoming edges
	   */
	  edge_iterator iebegin () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (incomingEdges.begin());}

	  /** 
	   *
	   * @return end iterator for incoming edges
	   */
	  edge_iterator ieend () {return SmartIterator<Edge_ptr,std::vector<Edge_wptr>::iterator> (incomingEdges.end());}

	  /** 
	   * Check if this location has outoing edges
	   *
	   * @return true if it has outgoing edges, false if not
	   */
      bool hasOutgoingEdge () const {
		return edges.size();
      }

      auto nbOutgoingEdges () const {
		return edges.size();
      }
      
      auto& getName () const {return name;}

	  

	  /** 
	   * Count the number of incoming edges
	   *
	   *
	   * @return number of incoming edges
	   */
      auto nbIncomingEdges () const  {
		return incomingEdges.size();
      }

	  /** 
	   * Check  if this location has Attributes \p i set 
	   *
	   *
	   * @return true if \p i  is set false otherwise
	   */
      template<Attributes i>
      bool is () {
		return static_cast<AttrType> (i) & flags;
      }

	  /** 
	   * Set attribute \p i.
	   *
	   */
      template<Attributes i>
      void set () {
		flags |= static_cast<AttrType> (i);
      }

	  /** 
	   * Remove attribute \p i from this Location.
	   *
	   */
      template<Attributes i>
      void unset () {
		flags &= ~static_cast<AttrType> (i);
      }

	  AttrType getAttributesFlags () const {return flags;}
	  void setAttributesFlags (AttrType t)  {flags = t;}
	  
	  
      auto getID () const {return id;}
      
	  bool isOutgoing (const MiniMC::Model::Edge_ptr& e) {
		auto it = std::find_if (edges.begin(),edges.end(),
								[&e](const Edge_wptr& ptr1) {
								  return ptr1.lock() == e;
								});
		return it != edges.end();
	  }

	  bool isIncoming  (const MiniMC::Model::Edge_ptr& e) {
		auto it = std::find_if (incomingEdges.begin(),incomingEdges.end(),
								[&e](const Edge_wptr& ptr1) {
								  return ptr1.lock() == e;
								});
		return it != incomingEdges.end();
	  }
	  
	protected:
	  
	  /** 
	   * Search for an edge \p e. If found delete it from the outgoing edges.
	   * Notice \p e is searched for using pointer equality.  
	   *
	   * @param e The edge to search for
	   */
      void removeEdge (const Edge_ptr e) {
		auto it = std::find_if (edges.begin(),edges.end(),
								[&e](const Edge_wptr& ptr1) {
								  return ptr1.lock() == e;
								});
		
		assert(it != edges.end());
		edges.erase (it);
      }

	  /** 
	   * Search for an edge \p e. If found delete it from the incoming  edges.
	   * Notice \p e is searched for using pointer equality.  
	   *
	   * @param e The edge to search for
	   */
      void removeIncomingEdge (const Edge_ptr e) {
		auto it = std::find_if (incomingEdges.begin(),incomingEdges.end(),
								[&e](const Edge_wptr& ptr1) {
								  return ptr1.lock() == e;
								});
		assert (it != incomingEdges.end());
		incomingEdges.erase (it);
		
      }
	  
	private:
      std::vector<Edge_wptr> edges;
      std::vector<Edge_wptr> incomingEdges;
      std::string name;
      AttrType flags = 0;
      MiniMC::offset_t id;
    };

	using Location_ptr = std::shared_ptr<Location>;
    using Location_wptr = std::weak_ptr<Location>;
	
	
  }
}

#endif 
