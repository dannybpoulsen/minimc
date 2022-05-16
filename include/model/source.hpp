#ifndef _SOURCE_INFO__
#define _SOURCE_INFO__

#include <memory>
#include <ostream>
#include <string>

#include "model/variables.hpp"
namespace MiniMC {
  namespace Model {

    using AttrType = char;
    /**
	 * The possible attributes that can be assigned locations
	 *
	 */

    struct SourceInfo : public std::enable_shared_from_this<SourceInfo> {
      virtual std::ostream& out(std::ostream& os) {
        return os << "[?]";
      }
    };

    using SourceInfo_ptr = std::shared_ptr<const SourceInfo>;

    enum class Attributes : AttrType {
      AssertViolated = 1,  /**< Indicates an assert was violated */
      NeededStore = 2,     /**< Indicates this location is part of loop, and must be stored for guaranteeing termination*/
      CallPlace = 4,       /**< Indicates a call takes place on an edge leaving this location */
      AssumptionPlace = 8, /**< Indicates an assumption is made on an edge leaving this location  */
      ConvergencePoint = 16,
      UnrollFailed = 32 /** Indicates loop unrolling was unsufficient **/
    };

    struct SourcePlace {
      const std::string filename;
    };

    struct LocationInfo {
      explicit LocationInfo(const std::string& name, AttrType flags, const SourceInfo& info, const MiniMC::Model::RegisterDescr* registers) : name(name), flags(flags), source(info.shared_from_this()),active_registers(registers) {}

      const std::string& getName() const { return name; }
      const RegisterDescr& getRegisters () const {return *active_registers;}
      /** 
	   * Check  if this location has Attributes \p i set 
	   *
	   *
	   * @return true if \p i  is set false otherwise
k	   */
      template <Attributes i>
      bool is() const {
        return static_cast<AttrType>(i) & flags;
      }

      /** 
	   * Set attribute \p i.
	   *
	   */
      template <Attributes i>
      void set() {
        flags |= static_cast<AttrType>(i);
      }

      /** 
	   * Remove attribute \p i from this Location.
	   *
	   */
      template <Attributes i>
      void unset() {
        flags &= ~static_cast<AttrType>(i);
      }

      bool isFlagSet(AttrType t) { return flags & t; }
      std::string name;
      AttrType flags;
      SourceInfo_ptr source;
      const MiniMC::Model::RegisterDescr* active_registers;
    };

    inline std::ostream& operator<<(std::ostream& os, const LocationInfo& info) {
      return os << info.getName();
    }

    struct LocationInfoCreator {
      LocationInfoCreator(const std::string prefix, const MiniMC::Model::RegisterDescr* regs) : pref(prefix),registers(regs) {}
      
      LocationInfo make(const std::string& name, AttrType type, const SourceInfo& info) {
        return LocationInfo(pref + ":" + name, type, info,registers);
      }

      LocationInfo make(const LocationInfo& loc) {
	
        return LocationInfo(pref + ":" + loc.name, loc.flags, *loc.source,loc.active_registers);
      }

    private:
      const std::string pref;
      const MiniMC::Model::RegisterDescr* registers;
    };

  } // namespace Model
} // namespace MiniMC

#endif
