#ifndef _SOURCE_INFO__
#define _SOURCE_INFO__

#include <string>
#include <ostream>

namespace MiniMC {
  namespace Model {

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
	
	struct LocationInfo   {
	  LocationInfo (const std::string& name, AttrType flags = 0) : name(name) {}
	  const std::string& getName () const {return name;}
	  
	  /** 
	   * Check  if this location has Attributes \p i set 
	   *
	   *
	   * @return true if \p i  is set false otherwise
	   */
      template<Attributes i>
      bool is () const {
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

	  
	private:
	  std::string name;
	  AttrType flags;
	};
	
	inline std::ostream& operator<< (std::ostream& os, const LocationInfo& info) {
	  return os << info.getName ();
	}
  }
}

#endif
