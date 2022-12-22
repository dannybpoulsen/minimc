#ifndef _SOURCE_INFO__
#define _SOURCE_INFO__

#include <memory>
#include <ostream>
#include <string>

#include "model/variables.hpp"
namespace MiniMC {
  namespace Model {

    class SourceInfo {
    public:
      struct SourceData {
	virtual ~SourceData () {}
	virtual std::ostream& out(std::ostream& os) = 0;
      };
      

      SourceInfo (SourceInfo&& ) =default;
      SourceInfo (const SourceInfo& ) =default;
      
      SourceInfo (std::shared_ptr<SourceData> data = nullptr) : data(data) {}
      std::ostream& out(std::ostream& os) {
	if (data) 
	  return data->out (os);
	else
	  os << "[?]";
      }
      
      
    private:
      std::shared_ptr<SourceData> data;
      
    };
    
    using AttrType = char;
    /**
     * The possible attributes that can be assigned locations
     *
     */
    
    
    enum class Attributes : AttrType {
      AssertViolated = 1,  /**< Indicates an assert was violated */
      
      UnrollFailed = 32 /** Indicates loop unrolling was unsufficient **/
    };

    struct LocationInfo {
      explicit LocationInfo(const Symbol& name, AttrType flags, MiniMC::Model::RegisterDescr registers, SourceInfo info = SourceInfo{})  : name(name), flags(flags), source(std::move(info)),active_registers(std::move(registers)) {}
      
      const std::string getName() const { return name.getFullName(); }
      const RegisterDescr& getRegisters () const {return active_registers;}
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
      Symbol name;
      AttrType flags;
      SourceInfo source;
      const MiniMC::Model::RegisterDescr active_registers;
    };

    inline std::ostream& operator<<(std::ostream& os, const LocationInfo& info) {
      return os << info.getName();
    }

    struct LocationInfoCreator {
      LocationInfoCreator(const Symbol prefix, const MiniMC::Model::RegisterDescr& regs) : pref(std::move(prefix)),registers(regs) {}
      
      LocationInfo make(const std::string& name, AttrType type, const SourceInfo& info) {
        return LocationInfo(Symbol{pref,name}, type, registers, info);
      }

      LocationInfo make(const std::string& name, AttrType type) {
        return LocationInfo(Symbol{pref,name}, type, registers);
      }
      
      LocationInfo make(const LocationInfo& loc) {
	
        return LocationInfo(Symbol{pref,loc.name.getName ()}, loc.flags, loc.active_registers, loc.source);
      }

    private:
      const Symbol pref;
      const MiniMC::Model::RegisterDescr registers;
    };

  } // namespace Model
} // namespace MiniMC

#endif
