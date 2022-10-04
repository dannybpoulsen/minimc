#ifndef MINIMC_PARSER_HPP
#define MINIMC_PARSER_HPP

#include "lexer.hpp"
#include "model/edge.hpp"
#include <istream>
#include <stack>
#include <string>

namespace MiniMC {
namespace Loader {

class Parser {
public:
  Parser(std::istream &is, Model::TypeFactory &tfac,
         Model::ConstantFactory &cfac, MiniMC::Model::Program_ptr& program)
      : in{&is}, tfac(tfac), cfac(cfac), prgm(program) {};

  void run();

private:
  std::istream *in;
  std::string buffer;
  Lexer* lexer;
  Model::TypeFactory &tfac;
  Model::ConstantFactory &cfac;
  MiniMC::Model::Program_ptr& prgm;

  void ignore_eol();
  void functions();
  void entrypoints();
  void heap();
  void initialiser();
  void function();
  Model::RegisterDescr_uptr registers(std::string name);
  void parameters(std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs);
  Model::Type_ptr returns();
  Model::CFA cfa(std::string name, std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs);
  void edge(std::string name, std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs, Model::CFA* cfa, std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap);
  Model::Location_ptr location(Model::CFA* cfg,std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap, std::shared_ptr<MiniMC::Model::SourceInfo> source_loc, MiniMC::Model::LocationInfoCreator locinfoc);
  void instruction(Model::InstructionStream* instructionStream, std::vector<MiniMC::Model::Register_ptr>* params);

  Model::Value_ptr value();
  Model::Register variable();
  std::string identifier();

};

}
}

#endif // MINIMC_PARSER_HPP
