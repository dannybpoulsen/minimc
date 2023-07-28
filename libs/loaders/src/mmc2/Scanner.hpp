#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__ 

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "tokens.hpp"

#include <regex>

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
	  switch (type) {
	  case HEXANUMBER:
	    *token = Token {type,std::stoll (yytext,nullptr,16)};
	    return NUMBER;
	  case NUMBER:
	    *token = Token {type,std::stoll (yytext,nullptr,10)};
	    return NUMBER;
	  case POINTERLITERAL:
	  {
	    std::regex literal_regex("([H|F])\\(([0-9]+)\\+([0-9]+)\\)");
	    std::cmatch m;
	    if (std::regex_match (yytext,m,literal_regex)) {
	      auto base = std::stoll (m[2].str());
	      auto offset = std::stoll (m[3].str());
	      if (m[1].str() == "H") {
		*token = Token {type,PointerLiteral{PointerSegment::Heap,base,offset}};
		
	      }

	      else if (m[1].str() == "F") {
		*token = Token {type,PointerLiteral{PointerSegment::Function,base,offset}};
		
	      }

	      else {
		throw MiniMC::Support::Exception ("Lexer error");
	      }
	      
	    }

	    return type;
	  }
	  default:
	    *token = Token {type,yytext};
	    return type;
	  }
	  
	}
	
	virtual int yylex(Token* lval);   
      private:
	Token* token;
      };
    }
  }
} /* end namespace MC */

#endif 
