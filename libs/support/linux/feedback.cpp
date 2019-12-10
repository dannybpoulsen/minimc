#include <iostream>
#include <gsl/pointers>
#include "support/feedback.hpp"
#include "colormod.hpp"
namespace MiniMC {
  namespace Support {
    class LinuxProgresser : public Progresser  {
    public:
      LinuxProgresser () {
      }

      ~LinuxProgresser () {
	std::cout << "\n";
      }

      virtual void progressMessage (const std::string& s) {
	std::cout << "\x1b[2K\r" << arr[next] << ' ' << s << std::flush;
	next = (next +1)% 4;
      }

    private:
      char arr[4] = {'\\', '|','/','-'};
      int next = 0;
    };
    

    
    class LinuxMessager : public Messager {
    public:
      LinuxMessager () : errorC(MiniMC::Linux::ColorModifier::Code::FG_RED),
			 warningC(MiniMC::Linux::ColorModifier::Code::FG_GREEN),
			 defaultC(MiniMC::Linux::ColorModifier::Code::FG_DEFAULT) {}
			 
      virtual Progresser_ptr makeProgresser () {return std::make_unique<LinuxProgresser> ();}
      virtual void error (const std::string& s) {
	std::cerr << errorC << "Error  :" << defaultC << s << std::endl;;
      }
      
      virtual void warning (const std::string& s) {
	std::cerr << warningC << "Warning:" << defaultC << s << std::endl;;
      }

      virtual void message (const std::string& s) {
	std::cerr << defaultC << "Message:" << defaultC << s << std::endl;;
      }
    private:
      MiniMC::Linux::ColorModifier errorC; 
      MiniMC::Linux::ColorModifier warningC;
      MiniMC::Linux::ColorModifier defaultC;
      
    };
    
    Messager_ptr makeMessager (MessagerType g) {
      switch (g) {
      case MessagerType::Terminal:
	return std::make_unique<LinuxMessager> ();
      default:
	return std::make_unique<LinuxMessager> ();
      }
    }
    
    
  }
}
