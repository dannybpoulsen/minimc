%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {MiniMC::Simulator}
%define api.parser.class {Parser}


%code requires{
    namespace MiniMC {
      namespace Simulator {
	class Scanner;
	class CommandBuilder;
      }
    }



#include <cstdint>
#include <string>
#include "minimc/io/ostream.hpp"
#include "minimc/model/variables.hpp"
    
 }

%parse-param { MiniMC::Simulator::Scanner& scanner } {MiniMC::Simulator::CommandBuilder& builder} {MiniMC::Model::ExpressionBuilder& exprbuilder}  {MiniMC::IO::ostream& messager}
%initial-action
{

};

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include "scanner.h"
  #include "commands.hpp"
#undef yylex
#define yylex scanner.yylex
  
}



%define api.value.type variant
%define parse.assert

%token    START_SIMULATION 
%token    SHOW_STATE
%token    SHOW_TRANSITIONS
%token    STEP
%token    SEARCH
%token    SYMBOLIC
%token    CEVAL
%token    SEVAL
%token    UI8
%token    UI16
%token    UI32
%token    UI64
%token    SI8
%token    SI16
%token    SI32
%token    SI64




%token <std::uint64_t> POS_NUMBER
%token <std::int64_t> NEG_NUMBER

%token END 0 "end of file"


%%

prgm : START_SIMULATION {builder.startSimulation ();} |
       SHOW_STATE {builder.showState();} |
       SHOW_TRANSITIONS {builder.showTransitions();} | 
       STEP {builder.step ();} |
       SEARCH {builder.search ();} |
       CEVAL expr {builder.evalExpression (exprbuilder.get());} |
       SEVAL expr {builder.sevalExpression (exprbuilder.get());}|
       error  {builder.skip ();}


expr :  UI8 POS_NUMBER {exprbuilder.I8 ($2);}
| UI16 POS_NUMBER {exprbuilder.I16 ($2);}
| UI32 POS_NUMBER {exprbuilder.I32 ($2);}
| UI64 POS_NUMBER {exprbuilder.I64 ($2);}
| SI8 POS_NUMBER {exprbuilder.I8 (($2));}
| SI16 POS_NUMBER {exprbuilder.I16 ( ($2));}
| SI32 POS_NUMBER {exprbuilder.I32 ( ($2));}
| SI64 POS_NUMBER {exprbuilder.I64 ( ($2));}
| SI8 NEG_NUMBER {exprbuilder.I8 (std::bit_cast<MiniMC::BV64> ($2));}
| SI16 NEG_NUMBER {exprbuilder.I16 (std::bit_cast<MiniMC::BV64> ($2));}
| SI32 NEG_NUMBER {exprbuilder.I32 (std::bit_cast<MiniMC::BV64> ($2));}
| SI64 NEG_NUMBER {exprbuilder.I64 (std::bit_cast<MiniMC::BV64> ($2));}
|  error  {exprbuilder.I8 (0);}

%%


void  MiniMC::Simulator::Parser::error(const std::string &err_message )
{
  messager << err_message;
}
