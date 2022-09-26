//
// Created by OA82HX on 23-09-2022.
//

#ifndef MINIMC_PARSER_HPP
#define MINIMC_PARSER_HPP

#include "cpa/interface.hpp"
#include "lexer.hpp"
#include <string>
namespace MiniMC {
namespace Interpreter {

class Parser {
public:
  Parser(std::unordered_map<std::string, CPA::AnalysisState> *statemap,
         CPA::AnalysisTransfer transfer)
      : statemap(statemap), transfer(transfer){};

  void operator()(const std::string& s)
  {
    std::istringstream ist{s};
    lexer = new Lexer{ist};
    command();
    delete lexer;
  }

private:
  Lexer *lexer;
  std::unordered_map<std::string, CPA::AnalysisState> *statemap;
  CPA::AnalysisTransfer transfer;

  void print();
  void jump();
  void step();
  void bookmark();
  std::string get_id();

  void command();

  MiniMC::Model::Edge* promptForEdge(MiniMC::CPA::AnalysisState);
  Model::Edge* haveNoInstructionEdge(CPA::AnalysisState);
};

} // namespace Interpreter
} // namespace MiniMC

#endif // MINIMC_PARSER_HPP
