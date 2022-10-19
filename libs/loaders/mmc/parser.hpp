#ifndef MINIMC_PARSER_HPP
#define MINIMC_PARSER_HPP

#include "lexer.hpp"
#include "model/edge.hpp"
#include <istream>
#include <stack>
#include <string>

namespace MiniMC {
namespace Loaders {

class Parser {
public:
  Parser(std::istream &is,Model::TypeFactory &tfac,
         Model::ConstantFactory &cfac , MiniMC::Model::Program_ptr program)
      : in{&is}, tfac(tfac), cfac(cfac), prgm(program) {};

  void run();

private:
  std::istream *in;
  Lexer* lexer;
  Model::TypeFactory &tfac;
  Model::ConstantFactory &cfac;
  MiniMC::Model::Program_ptr prgm;

  void ignore_eol();
  void functions();
  void entrypoints();
  void heap();
  void initialiser();
  void function();
  Model::RegisterDescr_uptr registers(Model::Symbol name);
  void parameters(std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs);
  Model::Type_ptr returns();
  Model::CFA cfa(Model::Symbol name, const MiniMC::Model::RegisterDescr* regs);
  void edge(Model::Symbol name, const MiniMC::Model::RegisterDescr* regs, Model::CFA* cfa, std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap);
  Model::Location_ptr location(Model::CFA* cfg,std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap, std::shared_ptr<MiniMC::Model::SourceInfo> source_loc, MiniMC::Model::LocationInfoCreator locinfoc);
  void instruction(Model::InstructionStream* instructionStream, std::vector<MiniMC::Model::Register_ptr> variables);
  Model::Instruction instruction_eq(const std::vector<MiniMC::Model::Register_ptr> variables);
  Model::Instruction predicates(const std::vector<MiniMC::Model::Register_ptr> variables);
  Model::Instruction tacops(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res);
  Model::Instruction comparison(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res);
  Model::Instruction castops(const std::vector<MiniMC::Model::Register_ptr> variables, Model::Value_ptr res) ;
  Model::Instruction nondet(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res);
  Model::Instruction extract(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res);
  Model::Instruction insert(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res);
  Model::Instruction load(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res);

  Model::Type_ptr type();
  std::vector<Model::Value_ptr> value_list(std::vector<MiniMC::Model::Register_ptr> variables);
  Model::Value_ptr value(std::vector<MiniMC::Model::Register_ptr> variables);
  Model::Register variable();
  Model::Symbol identifier();
  unsigned long int integer();
  std::vector<Model::Constant_ptr> integer_list();

};

}
}

#endif // MINIMC_PARSER_HPP
