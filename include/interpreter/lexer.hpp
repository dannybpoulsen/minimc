//
// Created by OA82HX on 23-09-2022.
//

#ifndef MINIMC_LEXER_HPP
#define MINIMC_LEXER_HPP

#include <istream>
namespace MiniMC {
namespace Interpreter {
enum class Token {
  BOOKMARK, JUMP, PRINT, STEP, ID, EOL
};

class Lexer {
  public:
    Lexer(std::istream& is);

    Token token() const {return cur_token; }
    void advance() {cur_token = get_token(); }
    std::string getTokenAsText(){ return buffer;}
  private:
    std::istream* in;
    Token cur_token;
    std::string buffer;

    Token get_token();
};

} // namespace Interpreter
} // namespace MiniMC

#endif // MINIMC_LEXER_HPP
