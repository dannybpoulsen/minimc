#include "minimc/loaders/loader.hpp"
#include "minimc/model/cfg.hpp"
#include "Scanner.hpp"
#include "parser.hpp"

#include <fstream>
#include <sstream>


namespace MiniMC {
namespace Loaders {


class MMCLoader2 : public Loader {
public:
  MMCLoader2(){}

  LoadResult loadFromFile(const std::string &file,  Model::ConstantFactory_ptr& cfac,MiniMC::Support::Messager&) override {
    std::fstream str;
    str.open(file);
    MiniMC::Loaders::MMC::Parser parser {str};
    MiniMC::Loaders::MMC::Token tt;

    return parser.parse (cfac);
  }
  LoadResult loadFromString(const std::string &inp,  Model::ConstantFactory_ptr&,MiniMC::Support::Messager&) override {
   MiniMC::Model::Program program;
   std::stringstream str;
   str.str(inp);
   return program;
  }
};

class MMCLoadRegistrar2 : public LoaderRegistrar {
public:
  MMCLoadRegistrar2()
      : LoaderRegistrar("MMC") {}

  Loader_ptr makeLoader() override {
    return std::make_shared<MMCLoader2>();
  }
};

  static MMCLoadRegistrar2 MMCloadregistrar;

} // namespace Loaders
} // namespace MiniMC
