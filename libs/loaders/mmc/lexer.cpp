#include "lexer.hpp"

namespace MiniMC {
namespace Loaders {
Token Lexer::get_token() {
  buffer.clear();

  while(char c = get_next_char()) {
    if(c == EOF)
      return Token::EOF_TOKEN;

    while (isspace(c)) {
      if (c == '\n')
        return Token::EOL_TOKEN;
      c = get_next_char();
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
        c = get_next_char();
        c = get_next_char();
        while (isalnum(c)) {
          buffer += c;
          c = get_next_char();
        }
        return Token::R_ARROW;
      }

      if (c == '0' && in->peek() == 'x') {
        buffer += c;
        c = get_next_char();
        buffer += c;
        c = get_next_char();
        while (isdigit(c)) {
          buffer += c;
          if(in->peek() == ')'){
            break;
          }
          c = get_next_char();
        }
        return Token::HEX;
      }
      if (c == '#' && in->peek() == '#') {
        buffer += c;
        c = get_next_char();
        buffer += c;
        return Token::HASHHASH_SIGN;
      }

      if (isdigit(c)) {
        buffer += c;
        if(in->peek() == ')'){
          return Token::DIGIT;
        }
        c = get_next_char();
        while (isdigit(c)) {
          buffer +=c;
          if(in->peek() == ')'){
            return Token::DIGIT;
          }
          c = get_next_char();
        }
        return Token::DIGIT;
      }

      while (isprint(c) && !isspace(c)) {
        buffer += c;
        if(in->peek() == '>'){
          break;
        }
        c = get_next_char();
      }

      if (keywordsMap.contains(buffer)) {
        return keywordsMap[buffer];
      } else {
        return Token::IDENTIFIER;
      }
    }
  }
  return Token::EOF_TOKEN;
}
char Lexer::get_next_char() {
  char c = in->get();
  if(c == '\n'){
    line++;
    pos = 1;
  } else {
    pos++;
  }
  return c;
}


} // namespace Loader
} // namespace MiniMC