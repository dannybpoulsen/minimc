#ifndef _PROMPTER__
#define _PROMPTER__

#include "minimc/io/ostream.hpp"

#include <iosfwd>
#include <string>
#include <span>

namespace MiniMC {
  namespace IO {
    class Prompter {
    public:
      virtual std::string readline () = 0;
      virtual std::size_t selectOption (std::span<std::string>) = 0;
      virtual void addHistory(const std::string&) = 0;
    };

    class StreamPrompter : public Prompter{
    public:
      StreamPrompter (std::istream& in, MiniMC::IO::ostream& out) : in(in),out(out) {}
      std::string readline () override;
      std::size_t selectOption (std::span<std::string>) override;
      void addHistory(const std::string&) override;
    private:
      std::istream& in;
      MiniMC::IO::ostream& out;
    };
  }
}


#endif
