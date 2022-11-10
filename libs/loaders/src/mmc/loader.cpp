#include "loaders/loader.hpp"
#include "parser.hpp"
#include "model/cfg.hpp"

#include <fstream>
#include <sstream>


namespace MiniMC {
namespace Loaders {


class MMCLoader : public Loader {
public:
  MMCLoader(Model::TypeFactory_ptr &tfac, Model::ConstantFactory_ptr &cfac)
      : Loader(tfac, cfac) {}

  LoadResult loadFromFile(const std::string &file) override {
    auto program =std::make_shared<MiniMC::Model::Program>(tfactory, cfactory);
    std::fstream str;
    str.open(file);
    Parser parser = Parser(str, *tfactory, *cfactory, program);
    parser.run();
    return {.program = program,
    };
  }
  LoadResult loadFromString(const std::string &inp) override {
    auto program = std::make_shared<MiniMC::Model::Program>(tfactory, cfactory);
    std::stringstream str;
    str.str(inp);
    Parser parser = Parser(str, *tfactory, *cfactory, program);
    parser.run();
    return {.program = program
            };
  }
};

class MMCLoadRegistrar : public LoaderRegistrar {
public:
  MMCLoadRegistrar()
      : LoaderRegistrar("MMC", {}) {}

  Loader_ptr makeLoader(MiniMC::Model::TypeFactory_ptr &tfac,
                        Model::ConstantFactory_ptr cfac) override {
    return std::make_unique<MMCLoader>(tfac, cfac);
  }
};

static MMCLoadRegistrar MMCloadregistrar;

} // namespace Loaders
} // namespace MiniMC
