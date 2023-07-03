#include "loaders/loader.hpp"
#include "model/cfg.hpp"
#include "Scanner.hpp"

#include <fstream>
#include <sstream>


namespace MiniMC {
namespace Loaders {


class MMCLoader2 : public Loader {
public:
  MMCLoader2(Model::TypeFactory_ptr &tfac, Model::ConstantFactory_ptr &cfac)
      : Loader(tfac, cfac) {}

  MiniMC::Model::Program loadFromFile(const std::string &file, MiniMC::Support::Messager&) override {
    MiniMC::Model::Program program(tfactory, cfactory);
    std::fstream str;
    str.open(file);
    MiniMC::Loaders::MMC::Scanner scanner {str};
    MiniMC::Loaders::MMC::Token tt;
    while(scanner.yylex(&tt) != 0) {
      std::cout << tt << std::endl;			   
    };
    /*Parser parser = Parser(str, *tfactory, *cfactory, program);
    parser.run();*/
    return program;
  }
  MiniMC::Model::Program loadFromString(const std::string &inp, MiniMC::Support::Messager&) override {
   MiniMC::Model::Program program (tfactory, cfactory);
   std::stringstream str;
   str.str(inp);
   /*Parser parser = Parser(str, *tfactory, *cfactory, program);
     parser.run();*/
   return program;
  }
};

class MMCLoadRegistrar2 : public LoaderRegistrar {
public:
  MMCLoadRegistrar2()
      : LoaderRegistrar("MMC2", {}) {}

  Loader_ptr makeLoader(MiniMC::Model::TypeFactory_ptr &tfac,
                        Model::ConstantFactory_ptr cfac) override {
    return std::make_unique<MMCLoader2>(tfac, cfac);
  }
};

static MMCLoadRegistrar2 MMCloadregistrar;

} // namespace Loaders
} // namespace MiniMC
