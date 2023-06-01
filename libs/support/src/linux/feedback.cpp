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
      
      void mess(const ErrorMessage& m) override {
	std::cerr << errorC << std::setw(8) << std::left << "Error" << defaultC;
	m.to_string(std::cerr) << std::endl;
      }

      void mess(const WarningMessage& m) override {
	std::cerr << warningC << std::setw(8) << std::left << "Warning" << defaultC;
	m.to_string(std::cerr) << std::endl;
      }

      void mess(const InfoMessage& m) override {
	std::cout << "\x1b[2K\r";
	m.to_string(std::cerr) << std::endl;
      }

      void mess(const ProgressMessage& m) override {
	static const char* arr = "\\|/-";
	static int i = 0;
	std::cout << "\x1b[2K\r" << "[" <<arr[i] << "]" << defaultC;
	m.to_string(std::cout) << '\r' << std::flush;
	i = (i+1) % 4; ;
	
      }
      
      
     

            
      
    private:
      MiniMC::Linux::ColorModifier errorC;
      MiniMC::Linux::ColorModifier warningC;
      MiniMC::Linux::ColorModifier defaultC;
      
    };

    std::shared_ptr<MessageSink> makeMessager(MessageSinkType g) {
      switch (g) {
        case MessageSinkType::Terminal:
          return std::make_unique<LinuxMessageSink>();
      default:
	return std::make_unique<LinuxMessageSink>();
      }
    }

  } // namespace Support
} // namespace MiniMC
