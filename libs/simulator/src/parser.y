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
    
 }

%parse-param { MiniMC::Simulator::Scanner& scanner } {MiniMC::Simulator::CommandBuilder& builder}  {MiniMC::IO::ostream& messager}
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



%token <std::int64_t> POS_NUMBER
%token <std::int64_t> NEG_NUMBER

%token END 0 "end of file"


%%

prgm : START_SIMULATION {builder.startSimulation ();} |
       SHOW_STATE {builder.showState();} |
       SHOW_TRANSITIONS {builder.showTransitions();} | 
       STEP {builder.step ();} |
       SEARCH {builder.search ();} |
       
       error  {builder.skip ();}

%%


void  MiniMC::Simulator::Parser::error(const std::string &err_message )
{
  messager << err_message;
}
