#include "lexer.hpp"
#include <istream>

namespace MiniMC {
namespace Interpreter {
Lexer::Lexer(std::istream &is) : in{&is} {
  advance();
}

Token Lexer::get_token() {
  buffer.clear();
  char c = in->get();

  while (isspace(c))
    c = in->get();
  if (!in)
    return Token::EOL;

  if (isdigit(c)) {
    buffer = c;
    c = in->get();
    while (isdigit(c)) {
      buffer += c;
      c = in->get();
    }
    return Token::NUMBER;
  }


  if (isalpha(c)) {
    buffer = c;
    c = in->get();

    while (isalnum(c)) {
      buffer += c;
      c = in->get();
    }
    if (commandMap.contains(buffer))
      return commandMap[buffer];
    else
      return Token::ID;
  }

  return Token::EOL;
}


}// namespace Interpreter
} // namespace MiniMC
