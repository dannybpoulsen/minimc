#include "loaders/loader.hpp"
#include "parser.hpp"
#include "model/cfg.hpp"

#include <fstream>
#include <sstream>


namespace MiniMC {
namespace Loaders {

MiniMC::Model::Function_ptr createEntryPoint(MiniMC::Model::Program& program, MiniMC::Model::Function_ptr function, std::vector<MiniMC::Model::Value_ptr>&&) {
  throw MiniMC::Support::ConfigurationException ("Loader does not support defining entry points");
}


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
            .entrycreator = createEntryPoint
    };
  }
  LoadResult loadFromString(const std::string &inp) override {
    auto program = std::make_shared<MiniMC::Model::Program>(tfactory, cfactory);
    std::stringstream str;
    str.str(inp);
    Parser parser = Parser(str, *tfactory, *cfactory, program);
    parser.run();
    return {.program = program,
            .entrycreator = createEntryPoint
            };
  }
};

class MMCLoadRegistrar : public LoaderRegistrar {
public:
  MMCLoadRegistrar()
      : LoaderRegistrar("MMC", {IntOption{.name = "stack",
                                          .description = "StackSize",
                                          .value = 200}}) {}

  Loader_ptr makeLoader(MiniMC::Model::TypeFactory_ptr &tfac,
                        Model::ConstantFactory_ptr cfac) override {
    auto stacksize = std::visit(
        [](auto &t) -> std::size_t {
          using T = std::decay_t<decltype(t)>;
          if constexpr (std::is_same_v<T, IntOption>)
            return t.value;
          else {
            throw MiniMC::Support::Exception("Horrendous error");
          }
        },
        getOptions().at(0));
    return std::make_unique<MMCLoader>(tfac, cfac);
  }
};

static MMCLoadRegistrar MMCloadregistrar;

} // namespace Loaders
} // namespace MiniMC
