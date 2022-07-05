#include "support/feedback.hpp"
#include "colormod.hpp"
#include <iostream>
#include <iomanip>
namespace MiniMC {
  namespace Support {
    class LinuxMessageSink : public MessageSink {
    public:
      LinuxMessageSink() : errorC(MiniMC::Linux::ColorModifier::Code::FG_RED),
			   warningC(MiniMC::Linux::ColorModifier::Code::FG_GREEN),
			   defaultC(MiniMC::Linux::ColorModifier::Code::FG_DEFAULT) {
      }

      ~LinuxMessageSink() {
        std::cerr << std::endl;
      }
      void error(const std::string& s) override {
        std::cerr <<  errorC << std::setw(8) <<std::left <<  "Error:" << defaultC << s << std::endl;
        ;
      }

      void warning(const std::string& s) override {
        std::cout << warningC << std::setw(8) << std::left << "Warning:" << defaultC << s << std::endl;
        ;
      }

      void message(const std::string& s) override {
        std::cout << "\x1b[2K\r" << defaultC << s << '\n';
      }

      void progress(const std::string& s) override {
	static const char* arr = "\\|/-";
	static int i = 0;
	std::cout << "\x1b[2K\r" << "[" <<arr[i] << "]" << defaultC << s << '\r' << std::flush;
	i = (i+1) % 4; ;
	
      }
      
      
    private:
      MiniMC::Linux::ColorModifier errorC;
      MiniMC::Linux::ColorModifier warningC;
      MiniMC::Linux::ColorModifier defaultC;
      
    };

    std::unique_ptr<MessageSink> makeMessager(MessagerType g) {
      switch (g) {
        case MessagerType::Terminal:
          return std::make_unique<LinuxMessageSink>();
      default:
	return std::make_unique<LinuxMessageSink>();
      }
    }

  } // namespace Support
} // namespace MiniMC
