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
	if constexpr (t != Severity::Progress)
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
      return false;
    }


    void ProgressStreamHandler::pump () {
      static const char prorg[] = "\\|/-";
      static int i = 0;
      stream << "\r" << MiniMC::IO::manipulator::clearline  << "["  << std::string{prorg[i]} << "] " << buffer.str() << "\r" <<  MiniMC::IO::manipulator::flush ;
      i = (i +1) % (sizeof(prorg)-1); 
    }class LinuxMessageSink : public MessageSink {
    public:
      LinuxMessageSink() : errorC(MiniMC::Linux::ColorModifier::Code::FG_RED),
			   warningC(MiniMC::Linux::ColorModifier::Code::FG_GREEN),
			   defaultC(MiniMC::Linux::ColorModifier::Code::FG_DEFAULT) {
      }

      ~LinuxMessageSink() {
        std::cerr << std::endl;
      }
      
      void errormess(const Message& m)  {
	//std::cerr << errorC << std::setw(8) << std::left << "Error" << defaultC << m << std::endl;
      }

      void warningmess(const Message& m)  {
	//std::cerr << warningC << std::setw(8) << std::left << "Warning" << defaultC << m << std::endl;
      }

      void infomess(const Message& m)  {
	//std::cout << "\x1b[2K\r" << m << std::endl;
      }
      
      void progressmess(const Message& m)  {
	std::stringstream str;
	//str << m;
	progress = str.str();
      }

      void pumpProgress () override {
	static const char arr[] = "\\|/-";
	static int i = 0;
	//std::cout << "\r\x1b[2K\r " << "[" <<arr[i] << "] " << defaultC;
	//std::cout << progress  << "\e7" << std::flush;
	i = (i+1) % (sizeof(arr)-1);
      }
      
      virtual MiniMC::IO::ostream& raw_stream (Severity) override {
	return MiniMC::IO::os_ostream::err();
      }
      
      void mess (const Message& m) override {
	switch (m.getType()) {
	case Severity::Info:
	  infomess(m);
	  break;
	case Severity::Error:
	  errormess(m);
	  break;
	case Severity::Warning:
	  warningmess(m);
	  break;
	case Severity::Progress:
	  progressmess(m);
	  break;
	}
      }
            
      
    private:
      MiniMC::Linux::ColorModifier errorC;
      MiniMC::Linux::ColorModifier warningC;
      MiniMC::Linux::ColorModifier defaultC;
      std::string progress;
    };

    std::shared_ptr<MessageSink> defSink = std::make_shared<LinuxMessageSink> ();
    
    void MessageSink::setDefaultSink (std::shared_ptr<MessageSink> d) {
      defSink = d;
    }
    
    
    std::shared_ptr<MessageSink> MessageSink::defaultSink () {
      return defSink;
    }

    template class StreamHandler<Severity::Info>;
    template class StreamHandler<Severity::Warning>;
    template class StreamHandler<Severity::Error>;
    template class StreamHandler<Severity::Progress>;
    
    
  } // namespace Support
} // namespace MiniMC
