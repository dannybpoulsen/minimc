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
hexa    "0x"[0-9A-F]+

number  {num1}|{num2}
name    [a-zA-Z\_][a-zA-Z0-9\_\.]+

%%
%{
token = lval;
%}
{ws}    /* skip blanks and tabs */
[\n] {}

"#"  {return makeToken (HASH);}
"<"  {return makeToken (LANGLE);}
">"  {return makeToken (RANGLE);}

{name} {return makeToken (IDENTIFIER);}
{name}([:]{name})+ {return makeToken (QUALIFIEDNAME);}
"{" {return makeToken (LBRACE);}
"}" {return makeToken (LBRACE);}
"[" {return makeToken (LBRACK);}
"]" {return makeToken (RBRACK);}

"@" {return makeToken (AT);}
"Int8" {return makeToken (INT8);}
"Int16" {return makeToken (INT16);}
"Int32" {return makeToken (INT32);}
"Int64" {return makeToken (INT64);}
"Void" {return makeToken (VOID);}
"Bool" {return makeToken (BOOL);}
"Aggr" {return makeToken (AGGR);}


".registers" {return makeToken (REGISTERS);}
".parameters" {return makeToken (PARAMETERS);}
".returns" {return makeToken (RETURNS);}
".cfa" {return makeToken (CFA);}
"=" {return makeToken (ASSIGN);}
{hexa} {return makeToken (HEXANUMBER);}


<<EOF>> {return END;}

%%

