#include "minimc/loaders/loader.hpp"
#include "minimc/model/cfg.hpp"
#include "whiley/ast.hpp"
#include "whiley/parser.hpp"
#include "whiley/typechecker.hpp"

#include "compiler.hpp"

#include <fstream>
#include <sstream>
#include <iostream>


namespace MiniMC {
namespace Loaders {


class WhileyLoader2 : public Loader {
public:
  WhileyLoader2(){}

  MiniMC::Model::Program loadFromFile(const std::string &file, MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr& cfac,MiniMC::Support::Messager&) override {
    ::Whiley::WParser parser;
    if (auto parseres = parser.parse (file)) {
      auto prgm = parseres.get();
      if (Whiley::TypeChecker{}.CheckProgram(prgm))
	return MiniMC::Loaders::whiley::Compiler {tfac,cfac}.compile(prgm);
    }
    return MiniMC::Model::Program{};
      
  }
  MiniMC::Model::Program loadFromString(const std::string &inp, MiniMC::Model::TypeFactory_ptr&, Model::ConstantFactory_ptr&,MiniMC::Support::Messager&) override {
   MiniMC::Model::Program program;
   return program;
  }
};

class WhileyLoadRegistrar : public LoaderRegistrar {
public:
  WhileyLoadRegistrar ()
      : LoaderRegistrar("Whiley") {}

  Loader_ptr makeLoader() override {
    return std::make_shared<WhileyLoader2>();
  }
};

  static WhileyLoadRegistrar whileyloadregistrar;

} // namespace Loaders
} // namespace MiniMC
