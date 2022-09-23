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

private:
  Lexer *lexer;
  std::unordered_map<std::string, CPA::AnalysisState> *statemap;
  CPA::AnalysisTransfer transfer;

  void print(std::string);
  void print();
  void jump(std::string);
  void jump();
  void step();
  void bookmark();
  void bookmark(std::string);
  std::string get_id();

  void primary();

  MiniMC::Model::Edge* promptForEdge(MiniMC::CPA::AnalysisState);
  Model::Edge* haveNoInstructionEdge(CPA::AnalysisState);
};

} // namespace Interpreter
} // namespace MiniMC

#endif // MINIMC_PARSER_HPP
