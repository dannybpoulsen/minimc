#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__ 

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "parser.hh"

namespace MiniMC  {
  namespace Simulator {
    
    class Scanner : public yyFlexLexer{
    public:
      
      Scanner(std::istream *in) : yyFlexLexer(in) {
      };
      
      using FlexLexer::yylex;
      
      virtual int yylex( MiniMC::Simulator::Parser::semantic_type * const lval);   
      
    private:
      /* yyval ptr */
      MiniMC::Simulator::Parser::semantic_type *yylval = nullptr;
    };
    
  }
} // namespace MiniMC  

#endif 
