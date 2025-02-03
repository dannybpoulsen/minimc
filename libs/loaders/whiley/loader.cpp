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

  LoadResult loadFromFile(const std::string &file,  Model::ConstantFactory_ptr& cfac,MiniMC::Support::Messager&) override {
    ::Whiley::WParser parser;
    if (auto parseres = parser.parse (file)) {
      auto prgm = parseres.get();
      if (Whiley::TypeChecker{}.CheckProgram(prgm))
	return MiniMC::Loaders::whiley::Compiler {cfac}.compile(prgm);
    }
    return MiniMC::Model::Program{};
      
  }
  LoadResult loadFromString(const std::string &inp,  Model::ConstantFactory_ptr&,MiniMC::Support::Messager&) override {
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
