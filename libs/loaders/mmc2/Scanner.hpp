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
	  Location loc{start,end};
	  switch (type) {
	  case HEXANUMBER:
	    *token = Token {NUMBER,std::stoll (yytext,nullptr,16),loc};
	    return NUMBER;
	  case NUMBER:
	    *token = Token {type,std::stoll (yytext,nullptr,10),loc};
	    return NUMBER;
	  case POINTERLITERAL:
	  {
	    std::regex literal_regex("([H|F])\\(([0-9]+)\\+([0-9]+)\\)");
	    std::cmatch m;
	    if (std::regex_match (yytext,m,literal_regex)) {
	      auto base = std::stoll (m[2].str());
	      auto offset = std::stoll (m[3].str());
	      if (m[1].str() == "H") {
		*token = Token {type,PointerLiteral{PointerSegment::Heap,base,offset},loc};
		
	      }

	      else if (m[1].str() == "F") {
		*token = Token {type,PointerLiteral{PointerSegment::Function,base,offset},loc};
		
	      }

	      else {
		throw MiniMC::Support::Exception ("Lexer error");
	      }
	    }
	    return type;	
	  }
	  
	  case AGGR: {
	    std::regex literal_regex("Aggr([0-9]+)");
	    std::cmatch m;
	    
	    if (std::regex_match (yytext,m,literal_regex)) {
	      auto size = std::stoll (m[1].str());
	      *token = Token {type,AggrType{size},loc};
	    }
	    else {
	      throw MiniMC::Support::Exception ("Lexer error");
	    }
	    return type;
	  }
	  
	  case AGGRCONSTANT: {
	    std::regex literal_regex("\\$(([0-9a-fA-F][0-9a-fA-F][ ])+)\\$");
	    std::cmatch m;
	    if (std::regex_match (yytext,m,literal_regex)) {
	      auto text = m[1].str ();
	      *token = Token {type,text,loc};
	    }
	    else {
	      throw MiniMC::Support::Exception ("Lexer error");
	    }
	    return type;
	  }
	  
	    
	  case NEWLINE:
	    end.newline ();
	    [[fallthrough]];
	  default:
	    *token = Token {type,yytext,loc};
	    return type;
	  }
	  
	}

	
	virtual int yylex(Token* lval);   
      private:
	void step () {
	  start = end;
	}
	Token* token;
	Cursor start;
	Cursor end;
      };
    }
  }
} /* end namespace MC */

#endif 
