#include "minimc/loaders/loader.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/program.hpp"
#include "whiley/ast.hpp"
#include "whiley/parser.hpp"
#include "whiley/typechecker.hpp"

#include "compiler.hpp"

#include <fstream>
#include <sstream>
#include <iostream>


namespace MiniMC {
namespace Loaders {


class WhileyLoader2 : public LoaderDirect {
public:
  WhileyLoader2() : LoaderDirect("whiley") {}

  LoadResult loadFromFile(const std::string &file,  MiniMC::Support::Messager& m) override {
    std::ifstream ifs;
    
    ifs.open (file, std::ifstream::in);
    return load(ifs,m);
  }
  LoadResult loadFromString(const std::string& s,  MiniMC::Support::Messager& m) override {
    std::stringstream str;
    str.str(s);
    return load(str,m);
  }

private:
  LoadResult load(std::istream& ff,  MiniMC::Support::Messager&)  {
    try {
      ::Whiley::WParser parser;
      if (auto parseres = parser.parse (ff)) {
	auto prgm = parseres.get();
	if (Whiley::TypeChecker{}.CheckProgram(prgm))
	  return MiniMC::Loaders::whiley::Compiler {}.compile(prgm);
      }
      return std::unexpected{Error::LoadFailed};
    }
    catch (std::runtime_error& e ) {
      std::cerr << e.what () << std::endl;
      return std::unexpected{Error::LoadFailed};
    }

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
