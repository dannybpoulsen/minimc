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
"step"       {return token::STEP;}
"search"       {return token::SEARCH;}
"symbolic"       {return token::SYMBOLIC;}



[0-9]+ {yylval->emplace<std::int64_t>(std::stoi(yytext)); return token::POS_NUMBER;}
-[1-9][0-9]*  {yylval->emplace<std::int8_t> (std::stoi(yytext)); return token::NEG_NUMBER; }


<<EOF>> {return token::END;}

			    
.           {}
%%
