%{
#include <iostream>
#include "Scanner.hpp"
int mylineno = 0;

#undef  YY_DECL
#define YY_DECL int MiniMC::Loaders::MMC::Scanner::yylex(MiniMC::Loaders::MMC::Token * const lval)
#define YY_USER_ACTION  end+=yyleng;
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
hexa    0x[0-9A-Fa-f]+

number  {num1}|{num2}
name    [a-zA-Z\_][a-zA-Z0-9\_\.-]*

%%
%{
token = lval;
step ();
%}
{ws}    /* skip blanks and tabs */
[\n] { return makeToken (NEWLINE);}


"##"  {return makeToken (DOUBLEHASH);}

"<"  {return makeToken (LANGLE);}
">"  {return makeToken (RANGLE);}
"->" {return makeToken (ARROW);}
[F|H]\({dig}+\+{dig}+\) {return makeToken (POINTERLITERAL);}

{name}([:]{name})+ {return makeToken (QUALIFIEDNAME);}
"{" {return makeToken (LBRACE);}
"}" {return makeToken (RBRACE);}
"[" {return makeToken (LBRACK);}
"]" {return makeToken (RBRACK);}

"@" {return makeToken (AT);}
"Int8" {return makeToken (INT8);}
"Int16" {return makeToken (INT16);}
"Int32" {return makeToken (INT32);}
"Int64" {return makeToken (INT64);}
"Void" {return makeToken (VOID);}
"Bool" {return makeToken (BOOL);}
"Pointer" {return makeToken (POINTER);}
Aggr[0-9]+ {return makeToken (AGGR);}
":"    {return makeToken (COLON);}

{name} {return makeToken (IDENTIFIER);}


".registers" {return makeToken (REGISTERS);}
".metaregisters" {return makeToken (METAREGISTERS);}
".parameters" {return makeToken (PARAMETERS);}
".returns" {return makeToken (RETURNS);}
".cfa" {return makeToken (CFA);}
"# Globals" {return makeToken (GLOBALS);}
"# Functions" {return makeToken (FUNCTIONS);}
"# Entrypoints" {return makeToken (ENTRYPOINTS);}
"# Heap" {return makeToken (HEAP);}
"# Initialiser" {return makeToken (INITIALISER);}

"=" {return makeToken (ASSIGN);}
{hexa} {return makeToken (HEXANUMBER);}
{number} {return makeToken (NUMBER);} 
\$([0-9A-Fa-f][0-9A-Fa-f][ ])+\$ {return makeToken(AGGRCONSTANT);}  

<<EOF>> {return END;}
.	printf("bad input character '%s'", yytext);
%%

