#include "lexer.hpp"

namespace MiniMC {
namespace Loaders {
Token Lexer::get_token() {
  buffer.clear();

  while(char c = in->get()) {
    if(c == EOF)
      return Token::EOF_TOKEN;

    while (isspace(c)) {
      if (c == '\n')
        return Token::EOL_TOKEN;
      c = in->get();
    }

    if (isprint(c)) {
      if(c == 'F' && in->peek() == '(')
        return Token::FUNCTION_POINTER;
      if(c == 'H' && in->peek() == '(')
        return Token::HEAP_POINTER;
      if(c == '(')
        return Token::L_PARA;
      if(c == ')')
        return Token::R_PARA;
      if(c == '+')
        return Token::PLUS_SIGN;

      if(c =='$')
        return Token::DOLLAR_SIGN;
      if (c == '<')
        return Token::LESS_THAN;
      if (c == '>')
        return Token::GREATER_THAN;

      if (c == '-' && in->peek() == '>') {
        c = in->get();
        c = in->get();
        while (isalnum(c)) {
          buffer += c;
          c = in->get();
        }
        return Token::R_ARROW;
      }

      if (c == '0' && in->peek() == 'x') {
        buffer += c;
        c = in->get();
        buffer += c;
        c = in->get();
        while (isdigit(c)) {
          buffer += c;
          c = in->get();
        }
        if(c == ')')
          in->unget();
        return Token::HEX;
      }
      if (c == '#' && in->peek() == '#') {
        buffer += c;
        c = in->get();
        buffer += c;
        return Token::HASHHASH_SIGN;
      }

      if (isdigit(c)) {
        buffer += c;
        c = in->get();
        while (isdigit(c)) {
          buffer +=c;
          c = in->get();
        }
        if(c == ')')
          in->unget();
        return Token::DIGIT;
      }

      while (isprint(c) && !isspace(c) && c != '>') {
        buffer += c;
        c = in->get();
      }
      if (c == '>')
        in->unget();
      if (keywordsMap.contains(buffer)) {
        return keywordsMap[buffer];
      } else {
        return Token::IDENTIFIER;
      }
    }
  }
  return Token::EOF_TOKEN;
}
} // namespace Loader
} // namespace MiniMC