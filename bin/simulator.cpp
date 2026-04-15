#include <boost/program_options.hpp>
#include "minimc/model/output.hpp"
#include "minimc/host/host.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/simulator/simulator.hpp"
#include "minimc/io/ostream.hpp"



#include "plugin.hpp"

namespace po = boost::program_options;




struct CLICommand : public Command{
  
  MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& prgm, MiniMC::Support::Interaction&,const SetupOptions&)  {
    auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
    MiniMC::Simulator::Simulator simulator {cpa,&prgm};
    MiniMC::IO::StreamPrompter prompter {std::cin,MiniMC::IO::os_ostream::out()};
    MiniMC::Simulator::CommandParser cmdparser {MiniMC::IO::os_ostream::out(),prompter,&prgm};
   
    std::unique_ptr<MiniMC::Simulator::Command> command;
    do {
      auto framegetter = [&prgm]([[maybe_unused]]std::size_t t) {return prgm.getRootFrame ();};
      command = cmdparser.parse (framegetter);
      if(command)
	command ->execute(&simulator);
    }while(command);
    
    
    return MiniMC::Host::ExitCodes::AllGood;
  }

  
  std::string getName () const override {return "cli";}
  std::string getDescritpion () const override {return "Simple CLI interface to MiniMC";}

};

static CommandRegistrar<CLICommand> cli_reg;
  
