
#ifndef MINIMC_PARSEREXCEPTION_HPP
#define MINIMC_PARSEREXCEPTION_HPP
#include <exception>
#include <sstream>

#include "support/localisation.hpp"
#include "lexer.hpp"
namespace MiniMC {
namespace Loaders {

class MMCParserException : public std::exception {
public:
  MMCParserException(int line, int pos, std::string value, std::string mesg) : line(line), pos(pos), value(value), mesg(mesg){}

  const int getPos() const {return pos;}
  const int getLine() const {return line;}
  const std::string getValue() const {return value;}
  const std::string getMesg() const {return mesg;}

private:
  int line;
  int pos;
  std::string value;
  std::string mesg;
};

} // namespace Loader
} // namespace MiniMC

#endif // MINIMC_PARSEREXCEPTION_HPP
