#ifndef MINIMC_LEXER_HPP
#define MINIMC_LEXER_HPP
#include <string>
#include <istream>
#include <map>

#include "tokens.hpp"

namespace MiniMC {
namespace Loaders {

class Lexer {
public:
  Lexer(std::istream &is) : in{&is} {
    line = 1;
    pos = 0;
    advance();
  }

  Token token() const {return cur_token; }
  void advance() {cur_token = get_token(); }
  std::string getValue(){ return buffer;}

  int getLine(){return line;}
  int getPos(){return pos;}
private:
  std::istream* in;
  Token cur_token;
  std::string buffer;
  int line;
  int pos;

  Token get_token();
  char get_next_char();
};

} // namespace Loader
} // namespace MiniMC

#endif // MINIMC_LEXER_HPP
