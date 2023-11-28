#include <boost/program_options.hpp>
#include "model/output.hpp"
#include "host/host.hpp"


#include "plugin.hpp"

namespace po = boost::program_options;



struct ConvCommand : public Command{
  MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& prgm, MiniMC::Support::Messager&,const SetupOptions&)  {
    MiniMC::Model::writeProgram (std::cout,prgm);
    
    return MiniMC::Host::ExitCodes::AllGood;
  }

  std::string getName () const override {return "convert";}
  std::string getDescritpion () const override {return "Output input program as MiniMC";}
  
  
};

static CommandRegistrar<ConvCommand> conv_reg;
  
