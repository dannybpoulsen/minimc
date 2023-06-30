%{
#include <iostream>
#include "Scanner.hpp"
int mylineno = 0;

#undef  YY_DECL
#define YY_DECL int MiniMC::Loaders::MMC::Scanner::yylex(MiniMC::Loaders::MMC::Token * const lval)

%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="MiniMC::Loaders::MMC::Scanner"


string  \"[^\n"]+\"
ws      [ \t]+
alpha   [A-Za-z]
dig     [0-9]
num1    [-+]?{dig}+\.?([eE][-+]?{dig}+)?
num2    [-+]?{dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}
name    [a-zA-Z][a-zA-Z0-9]+

%%
%{
token = lval;
%}
{ws}    /* skip blanks and tabs */
[\n] {}

"#"  {return makeToken (HASH);}
{name} {return makeToken (IDENTIFIER);}
{name}([:]{name})+ {return makeToken (QUALIFIEDNAME);}

".registers" {return makeToken (REGISTERS);}
".parameters" {return makeToken (PARAMETERS);}
".returns" {return makeToken (RETURNS);}
".cfa" {return makeToken (CFA);}

<<EOF>> {return END;}

%%

int main( int /* argc */, char** /* argv */ )
    {
    MiniMC::Loaders::MMC::Scanner lexer{std::cin};
    MiniMC::Loaders::MMC::Token tt;
    while(lexer.yylex(&tt) != 0) {
    	std::cout << tt.text << std::endl;			   
    };
    return 0;
    }