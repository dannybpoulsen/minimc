%{
/* C++ string header, for string ops below */
#include <string>
#include "scanner.h"
/* Implementation of yyFlexScanner */ 
#undef  YY_DECL
#define YY_DECL int MiniMC::Simulator::Scanner::yylex(MiniMC::Simulator::Parser::semantic_type * const lval)

/* using "token" to make the returns for the tokens shorter to type */
using token = MiniMC::Simulator::Parser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return( token::END )

%}

%option debug
%option nodefault
%option yyclass="MiniMC::Support::Scanner"
%option noyywrap
%option yylineno
%option c++
%option prefix="ss"

%%
%{
  yylval = lval;
%}
"run"      {return token::START_SIMULATION;}
"show"      {return token::SHOW_STATE;}
"transitions"      {return token::SHOW_TRANSITIONS;}



<<EOF>> {return token::END;}

			    
.           {}
%%
