%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {MiniMC::Simulator}
%define api.parser.class {Parser}


%code requires{

    

#include <cstdint>
#include <string>
#include <functional>
#include "minimc/model/symbol.hpp"
#include "minimc/io/ostream.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/simulator/parsercontext.hpp"
#include "minimc/smt/smt.hpp"
     
 }

%parse-param { MiniMC::Simulator::Scanner& scanner } {ParserContext context}
%initial-action
{
};

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include "scanner.h"
  #include "commands.hpp"
  #include "minimc/smt/smt.hpp"
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
%token    LPARAN
%token    RPARAN
%token    LBRACE
%token    RBRACE

%token    ADD
%token    SUB
%token    MUL


%token BV8
%token BV16
%token BV32
%token BV64
%token POINTER
%token AGGR
%token LOAD


%token    <std::string> SYMBOL_STRING



%token <std::uint64_t> POS_NUMBER
%token <std::int64_t> NEG_NUMBER

%token END 0 "end of file"


%%

prgm : START_SIMULATION {context.builder.startSimulation ();} |
       SHOW_STATE {context.builder.showState();} |
       SHOW_TRANSITIONS {context.builder.showTransitions();} | 
       STEP {context.builder.step ();} |
       SEARCH {context.builder.search ();} |
       SYMBOLIC SYMBOL_STRING {
  auto smt = MiniMC::Support::SMT::SMTSolverRepository::get().getBackend ($2);
  if (smt) {
    context.builder.makeSymbolic(smt.value());
  }
  else {
    context.os << "Unknown SMT Solver " << $2;
    context.builder.skip();
  }
       } |
       SYMBOLIC {context.builder.makeSymbolic();} | 
       CEVAL proc_expr {context.builder.evalExpression (context.ebuilder.get());} |
       SEVAL proc_expr {context.builder.sevalExpression (context.ebuilder.get());}|
       
       error  {context.builder.skip ();}

proc_expr : POS_NUMBER { context.proc = $1; } expr

expr :  UI8 POS_NUMBER {context.ebuilder.I8 ($2);}
| UI16 POS_NUMBER {context.ebuilder.I16 ($2);}
| UI32 POS_NUMBER {context.ebuilder.I32 ($2);}
| UI64 POS_NUMBER {context.ebuilder.I64 ($2);}
| SI8 POS_NUMBER {context.ebuilder.I8 (($2));}
| SI16 POS_NUMBER {context.ebuilder.I16 ( ($2));}
| SI32 POS_NUMBER {context.ebuilder.I32 ( ($2));}
| SI64 POS_NUMBER {context.ebuilder.I64 ( ($2));}
| SI8 NEG_NUMBER {context.ebuilder.I8 (std::bit_cast<MiniMC::BV64> ($2));}
| SI16 NEG_NUMBER {context.ebuilder.I16 (std::bit_cast<MiniMC::BV64> ($2));}
| SI32 NEG_NUMBER {context.ebuilder.I32 (std::bit_cast<MiniMC::BV64> ($2));}
| SI64 NEG_NUMBER {context.ebuilder.I64 (std::bit_cast<MiniMC::BV64> ($2));}
| LPARAN ADD expr expr RPARAN {context.ebuilder.Add ();}
| LPARAN SUB expr expr RPARAN {context.ebuilder.Sub ();}
| LPARAN MUL expr expr RPARAN {context.ebuilder.Mul ();}
| LPARAN LOAD expr expr type RPARAN {context.ebuilder.Load ();
}
| SYMBOL_STRING {
  MiniMC::Model::Symbol symb;
  if (context.getter (context.proc).resolveQualified ($1,symb)) {
    context.ebuilder.symbol(symb);
  }
  else
    context.ebuilder.I64(0);

    }
|  error  {context.ebuilder.I8 (0);}

type : BV8 {context.ebuilder.pushI8Type ();}
| BV16 {context.ebuilder.pushI16Type ();}
| BV32 {context.ebuilder.pushI32Type ();}
| BV64 {context.ebuilder.pushI64Type ();}
| POINTER {context.ebuilder.pushPointerType ();}
| AGGR POS_NUMBER {context.ebuilder.pushAggregateType ($2);}


%%


void  MiniMC::Simulator::Parser::error(const std::string &err_message )
{
  context.os << err_message;
}
