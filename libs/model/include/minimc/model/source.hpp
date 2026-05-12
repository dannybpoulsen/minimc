#ifndef _SOURCE_INFO__
#define _SOURCE_INFO__

#include <memory>
#include <ostream>
#include <string>

#include "minimc/model/variables.hpp"
#include "minimc/model/flags.hpp"

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
    
    
    enum class Attributes {
      AssertViolated = 1 << 0,  /**< Indicates an assert was violated */
      
      UnrollFailed = 1 << 1, /** Indicates loop unrolling was unsufficient **/
      Committed = 1 << 2
    };
    
    using LocFlags = FlagSet<Attributes>;
    
    struct LocationInfo {
      explicit LocationInfo(LocFlags flags, MiniMC::Model::RegisterDescr registers, MiniMC::Model::Frame frame,  SourceInfo info = SourceInfo{})  : flags(flags), source(std::move(info)),active_registers(std::move(registers)),frame(frame) {}
      LocationInfo (const LocationInfo&) = default;

      
      
      const RegisterDescr& getRegisters () const {return active_registers;}
      const auto& getFrame () const {return frame;}
      auto& getFlags () const  {return flags;} 
      auto& getFlags () {return flags;} 
      
      LocFlags flags;
      SourceInfo source;
      const MiniMC::Model::RegisterDescr active_registers;
      MiniMC::Model::Frame frame;
    };
    
    struct LocationInfoCreator {
      LocationInfoCreator(const MiniMC::Model::RegisterDescr& regs, MiniMC::Model::Frame frame) : registers(regs),frame(frame) {}
      
      LocationInfo make(LocFlags type, const SourceInfo& info = {}) {
        return LocationInfo(type, registers, frame, info);
      }

      
      /*LocationInfo make(const LocationInfo& loc) {	
        return LocationInfo(loc.flags, loc.active_registers, loc.source);
	}*/

    private:
      const MiniMC::Model::RegisterDescr registers;
      MiniMC::Model::Frame frame;
    };
    
  } // namespace Model
} // namespace MiniMC

#endif
