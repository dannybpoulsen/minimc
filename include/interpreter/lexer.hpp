#ifndef MINIMC_LEXER_HPP
#define MINIMC_LEXER_HPP

#include <istream>
#include <map>
#include <iterator>

namespace MiniMC {
namespace Interpreter {
enum class Token {
  BOOKMARK, JUMP, PRINT, STEP, ID, EOL, HELP, EDGES, EDGE, NUMBER
};

class Lexer {
  public:
    Lexer(std::istream& is);

    Token token() const {return cur_token; }
    void advance() {cur_token = get_token(); }
    std::string getTokenAsText(){ return buffer;}

    std::map<std::string,Token> commandMap = {
        {"print", Token::PRINT},
        {"jump", Token::JUMP},
        {"bookmark", Token::BOOKMARK},
        {"step", Token::STEP},
        {"edges", Token::EDGES},
        {"help", Token::HELP}
    };

  private:
    std::istream* in;
    Token cur_token;
    std::string buffer;


    Token get_token();
};

} // namespace Interpreter
} // namespace MiniMC

#endif // MINIMC_LEXER_HPP
