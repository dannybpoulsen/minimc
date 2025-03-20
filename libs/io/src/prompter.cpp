#include "minimc/io/prompter.hpp"

#include <istream>
#include <string>
#include <malloc.h>
#include <readline/readline.h>
#include <readline/history.h>

namespace MiniMC {
  namespace IO {
    std::string StreamPrompter::readline ()  {
      char* buf = ::readline (">");
      if (buf) {
	std::string res (buf);
	free(buf);
	return res;
      }
      return std::string{};
    }

    void  StreamPrompter::addHistory (const std::string&s ) {
      add_history(s.c_str());
    }

    std::size_t StreamPrompter::selectOption (std::span<std::string> range) {
      std::size_t i = 0;
      out << "Select Option\n" ;
      for (auto s : range) {
	out << (i++) << " : " << s << "\n"; 
      }
      out << MiniMC::IO::manipulator::flush;
      std::size_t selection = 0;
      do {
	std::stringstream str {readline ()};
	str >> selection;
	if (selection > range.size())  {
	  out << "Invalid Selection\n" ;
      
	}
      }while(selection > range.size());
      return selection;
    }
      
    
  }
}
