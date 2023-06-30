#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__ 

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "tokens.hpp"

namespace MiniMC {
  namespace  Loaders {
    namespace MMC {
      class Scanner : public yyFlexLexer{
      public:
	
	Scanner(std::istream& in) : yyFlexLexer(&in) {
	  //loc = new FMTeach::Whiley::Parser::location_type();
	};
	
	//get rid of override virtual function warning
	using FlexLexer::yylex;

	int makeToken (TokenType type) {
	  *token = Token {type,yytext};
	  return type;
	}
	
	virtual int yylex(Token* lval);   
      private:
	Token* token;
      };
    }
  }
} /* end namespace MC */

#endif 
