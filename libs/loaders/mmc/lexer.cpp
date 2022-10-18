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
      auto il = {c, (char) in->peek()};

      if(twosignsymbolMap.contains(std::string(il))){
        Token token = twosignsymbolMap[std::string(il)];
        if(token == Token::R_ARROW){
          c = get_next_char();
          while (isalnum(c)) {
            buffer += c;
            c = get_next_char();
          }
        } else if (token == Token::HEX){
          buffer += c;
          c = get_next_char();
          buffer += c;
          c = get_next_char();
          while (isxdigit(c)) {
            buffer += c;
            if(in->peek() == ')'){
              break;
            }
            c = get_next_char();
          }
        } else {
          buffer += c;
          c = get_next_char();
          buffer += c;
        }
        return token;
      }

      if (symbolsMap.contains(c)) {
        return symbolsMap[c];
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

  if (c == '/'){
    if (c == '/'){
      while(c != '\n')
        c = in->get();
    } else if (c == '*'){
      while(c != '*' && in->peek()!= '/')
        c = in->get();
    }
  }


  return c;
}


} // namespace Loader
} // namespace MiniMC