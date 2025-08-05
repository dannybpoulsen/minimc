#include "minimc/support/feedback.hpp"
#include "colormod.hpp"
#include <iostream>
#include <iomanip>
namespace MiniMC {
  namespace Support {

    template<Severity t>
    void StreamHandler<t>::pump () { 
      //stream.flush();
    }
    
    template<Severity T>
    struct SeverData {
      static MiniMC::Linux::ColorModifier color();
      static const std::string text (); 
    };

    template<>
    struct SeverData<Severity::Error> {
      static const MiniMC::IO::ColorMod color() {return {MiniMC::IO::ColorMod::Color::RED};}
      static const std::string text () {return "Error" ;}
    };
    
    template<>
    struct SeverData<Severity::Warning> {
      static const MiniMC::IO::ColorMod color() {return {MiniMC::IO::ColorMod::Color::GREEN};}
      static const std::string text () {return "Warning ";}
    };

    template<>
    struct SeverData<Severity::Info> {
      static const MiniMC::IO::ColorMod color() {return {MiniMC::IO::ColorMod::Color::BLUE};}
      static const std::string text () {return "";}
    };

    template<>
    struct SeverData<Severity::Progress> {
      static const MiniMC::IO::ColorMod color() {return {MiniMC::IO::ColorMod::Color::BLUE};}
      static const std::string text () {return "\33[2K\r";}
    };

    template<Severity t>
    bool StreamHandler<t>::handle(const Message& m) {
      if (m.getType() == t) {
	stream << MiniMC::IO::manipulator::clearline << SeverData<t>::color ()  <<  SeverData<t>::text () << MiniMC::IO::ColorMod (MiniMC::IO::ColorMod::Color::DEFAULT) <<  ""  << m <<"\n";
	if constexpr (t != Severity::Progress && t != Severity::SubProgress)
	  stream << MiniMC::IO::manipulator::flush;
	return true;
      }
      return false;
    }

    bool ProgressStreamHandler::handle(const Message& m) {
      if (m.getType()== Severity::Progress) {
	buffer.clear();
	buffer << m; 
	return true;
	
      }
      else if (m.getType() == Severity::SubProgress) {
	sub_message.clear();
	sub_message << m; 
	return true;
	
      }
      return false;
    }


    void ProgressStreamHandler::pump () {
      static const char prorg[] = "\\|/-";
      static int i = 0;
      stream << "\r" << MiniMC::IO::manipulator::clearline  << "["  << std::string{prorg[i]} << "] " << buffer.str() << " ( " << sub_message.str() <<" )"<< "\r" <<  MiniMC::IO::manipulator::flush ;
      i = (i +1) % (sizeof(prorg)-1); 
    }

    template class StreamHandler<Severity::Info>;
    template class StreamHandler<Severity::Warning>;
    template class StreamHandler<Severity::Error>;
    
    
  } // namespace Support
} // namespace MiniMC
