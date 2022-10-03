#ifndef MINIMC_PARSER_HPP
#define MINIMC_PARSER_HPP

#include "../llvm/llvmpasses.hpp"
#include "lexer.hpp"
#include "model/edge.hpp"
#include <istream>
#include <stack>
#include <string>

namespace MiniMC {
namespace Loader {

struct Variable{
  std::string value;
  Model::Type_ptr type;
};

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
  void hash_sign();
  void functions();
  void hashhash_sign();
  void function();
  Model::RegisterDescr_uptr registers(std::string name);
  struct Variable variable();
  std::string identifier();
  std::string hex();
  Model::Type_ptr type();
  void parameters(std::vector<Model::Register_ptr> registers, std::vector<Model::Register_ptr>* params);
  void returns(Model::Type_ptr retType);
  void cfa(Model::CFA*,MiniMC::Model::RegisterDescr*);
  Model::Instruction instruction();
  Model::Instruction single_instr(std::vector<std::pair(Token,std::string)> tokens);

  void entrypoints();
  void heap();
  void intialiser();



};

}
}

#endif // MINIMC_PARSER_HPP
