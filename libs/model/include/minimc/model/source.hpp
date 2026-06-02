#ifndef _SOURCE_INFO__
#define _SOURCE_INFO__

#include <memory>
#include <ostream>
#include <string>

#include "minimc/model/variables.hpp"
#include "minimc/model/flags.hpp"
#include "minimc/io/ostream.hpp"

namespace MiniMC {
  namespace Model {

    class SourceInfo {
    public:
      struct SourceData {
	virtual ~SourceData () {}
	virtual std::ostream& out(std::ostream& os) = 0;
	virtual MiniMC::IO::ostream& output(MiniMC::IO::ostream& os) const = 0;
	
      };

      template<class T,class... Args >
      static SourceInfo make(Args... args) {return SourceInfo (std::make_shared<T> (args...));}
      
      SourceInfo (SourceInfo&& ) =default;
      SourceInfo (const SourceInfo& ) =default;
      
      SourceInfo (std::shared_ptr<SourceData> data = nullptr) : data(data) {}

      SourceInfo& operator= (const SourceInfo&)  = default;
      
      
      std::ostream& out(std::ostream& os) {
	if (data) 
	  return data->out (os);
	else
	  return os << "[?]";
      }

      virtual MiniMC::IO::ostream& output(MiniMC::IO::ostream& os) const {
	if (data) 
	  return data->output (os);
	else
	  return os << "[?]";
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
      explicit LocationInfo(LocFlags flags, MiniMC::Model::Frame frame,  SourceInfo info = SourceInfo{})  : flags(flags), source(std::move(info)),frame(frame) {}
      LocationInfo (const LocationInfo&) = default;
      
      const auto& getFrame () const {return frame;}
      auto& getFlags () const  {return flags;} 
      auto& getFlags () {return flags;}
      SourceInfo getSource () const {return source;}
      
      LocFlags flags;
      SourceInfo source;
      MiniMC::Model::Frame frame;
    };
    
    struct LocationInfoCreator {
      LocationInfoCreator(MiniMC::Model::Frame frame) : frame(frame) {}
      LocationInfoCreator()  {}
      
      LocationInfo make() {
        return LocationInfo(flags, frame, info);
      }

      template<Attributes attr>
      void setFlag () {flags |= attr;}

      template<Attributes attr>
      void unsetFlag () {flags.unset(attr);}

      void setSource (const SourceInfo& info) { this->info = info;}
      
      void setFrame(MiniMC::Model::Frame frame) {this->frame = frame;}

      void setFlags (LocFlags f) {flags = f;}
      
    private:
      MiniMC::Model::Frame frame;
      LocFlags flags;
      SourceInfo info;
    };

    struct InfoResetter {
      InfoResetter (LocationInfoCreator& c) : infoc(&c),info(c.make()) {}
      ~InfoResetter() {
	infoc->setSource(info.getSource ());
	infoc->setFlags (info.getFlags());
	infoc->setFrame (info.getFrame());
	
      }
    private:
      LocationInfoCreator* infoc;
      LocationInfo info;
    };
    
  } // namespace Model
} // namespace MiniMC

#endif
