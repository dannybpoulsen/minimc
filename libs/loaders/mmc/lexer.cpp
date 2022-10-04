#include "lexer.hpp"

namespace MiniMC {
namespace Loader {
Token Lexer::get_token() {
  buffer.clear();
  while(char c = in->get()){
    if(c =='\n') return Token::EOL_TOKEN;

    while (isspace(c))
      c = in->get();

    if(isprint(c)){
      buffer = c;

      if(c=='-' && in->peek() == '>'){
        c = in->get();
        c = in->get();
        buffer = c;
        while(isalnum(c)){
          buffer += c;
          c = in->get();
        }
        return Token::R_ARROW;
      }

      if(c=='0' && in->peek() == 'x'){
        c = in->get();
        buffer += c;
        c = in->get();
        while(isdigit(c)){
          buffer += c;
          c = in->get();
        }
        return Token::HEX;
      }
      if(c=='#' && in->peek() == '#'){
        c = in->get();
        buffer += c;
        c = in->get();
        return Token::HASHHASH_SIGN;
      }

      if(isdigit(c)) {
        buffer += c;
        c = in->get();
        while (isdigit(c)) {
          buffer += c;
          c = in->get();
        }
        return Token::DIGIT;
      }

      while(isprint(c) && !isspace(c)) {
        buffer += c;
        c = in->get();

        if (keywordsMap.contains(buffer)) {
          return keywordsMap[buffer];
        } else {
          return Token::IDENTIFIER;
        }
      }
    }
  }
  return Token::EOF_TOKEN;
}
} // namespace Loader
} // namespace MiniMC