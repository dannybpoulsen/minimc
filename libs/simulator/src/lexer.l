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
"eval"       {return token::CEVAL;}
"seval"       {return token::SEVAL;}
"ui8"   {return token::UI8;}
"ui16"   {return token::UI16;}
"ui32"   {return token::UI32;}
"ui64"   {return token::UI32;}
"si8"   {return token::SI8;}
"si16"   {return token::SI16;}
"si32"   {return token::SI32;}
"si64"   {return token::SI64;}



[0-9]+ {yylval->emplace<std::uint64_t>(std::stoi(yytext)); return token::POS_NUMBER;}
-[1-9][0-9]*  {yylval->emplace<std::int64_t> (std::stoi(yytext)); return token::NEG_NUMBER; }


<<EOF>> {return token::END;}

			    
.           {}
%%
