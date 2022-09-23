//
// Created by OA82HX on 23-09-2022.
//

#include "interpreter/lexer.hpp"
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

  if (isalpha(c)) {
    buffer = c;
    c = in->get();

    while (isalnum(c)) {
      buffer += c;
      c = in->get();
    }
    if (buffer == "print")
      return Token::PRINT;
    else if (buffer == "jump")
      return Token::JUMP;
    else if (buffer == "bookmark")
      return Token::BOOKMARK;
    else if (buffer == "step")
      return Token::STEP;
    else
      return Token::ID;
  }
}


}// namespace Interpreter
} // namespace MiniMC