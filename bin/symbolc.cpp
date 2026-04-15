#include <boost/program_options.hpp>
#include "minimc/model/output.hpp"
#include "minimc/host/host.hpp"


#include "plugin.hpp"

#include <sstream>
#include <format>

namespace po = boost::program_options;


std::string symbol_descr_text (const MiniMC::Model::Symbol& s) {
  std::stringstream str;

  std::visit (MiniMC::Support::Overload {
      [&str](const MiniMC::Model::Register_wptr&) {str << "Register";},
      [&str](const MiniMC::Model::Function_wptr&) {str << "Function";},
      [&str](const MiniMC::Model::Location_wptr&) {str << "Location";},
      [&str](const MiniMC::Model::HeapBlock_wptr&) {str << "HeapObject";},
      [&str](const std::monostate& ) {str << "??";}
	}
    ,s.getUserData());
  return str.str();
}


struct ConvCCommand : public Command{
  MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& prgm, MiniMC::Support::Interaction&,const SetupOptions&)  {

    for (auto s : prgm.getRootFrame().symbols () ) {
      std::cerr << std::format ("{:20} {:20} ", s.getFullName(), symbol_descr_text(s)) << std::endl;
    }
    
    return MiniMC::Host::ExitCodes::AllGood;
  }

  std::string getName () const override {return "symbols";}
  std::string getDescritpion () const override {return "Dump symbols of the program";}
  
  
};

static CommandRegistrar<ConvCCommand> symbols_reg;

