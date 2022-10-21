#include "model/controller.hpp"
#include "model/output.hpp"
#include "plugin.hpp"
#include "support/localisation.hpp"
#include <boost/program_options.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC
#include "cpa/pathformula.hpp"
#endif
#include "cpa/concrete.hpp"
#include "loaders/loader.hpp"
#include "options.hpp"

namespace po = boost::program_options;

void transformProgram (auto& controller, const transform_options& options) {
  controller.createAssertViolateLocations();      
  if (options.expand_nondet) {
    controller.expandNonDeterministic ();
  }
}

int main(int argc, char* argv[]) {
  
  std::string input;
  std::string subcommand;
  MiniMC::Support::setMessageSink(MiniMC::Support::MessagerType::Terminal);
  MiniMC::Support::Messager messager;
  try {
    
    SetupOptions options;
    bool ok = parseOptions(argc, argv, options);
    
    if (ok) {
      // Load Program
      MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64>();
      MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64>(tfac);
      auto loader = options.load.registrar->makeLoader  (tfac,cfac);
      MiniMC::Loaders::LoadResult loadresult = loader->loadFromFile (options.load.inputname);
      
      MiniMC::Model::Controller control(*loadresult.program,loadresult.entrycreator);
      control.boolCasts();
  
      if (!control.typecheck ()) {
	return -1;
      }
      transformProgram (control,options.transform);
      
      
      for (std::string& s : options.load.tasks) {
	try {
	  control.addEntryPoint(s, {});
	} catch (MiniMC::Support::FunctionDoesNotExist&) {
	  messager.message<MiniMC::Support::Severity::Error> (MiniMC::Support::Localiser{"Function '%1%' specicifed as entry point does not exists. "}.format(s));
	  return -1;
	  }
      }
      
      
      if (options.outputname != "") {
	std::ofstream stream;
	stream.open (options.outputname, std::ofstream::out);
	MiniMC::Model::writeProgram (stream,*control.getProgram ());
	stream.close ();
      }
      if (options.command) {
	auto res =  static_cast<int>(options.command->getFunction()(control,options.cpa));
      
      return res;
      }
      
      else {
	messager.message<MiniMC::Support::Severity::Error> ("No subcommand selected");
	
      return static_cast<int>(MiniMC::Host::ExitCodes::ConfigurationError);
      }
    }
  }
  catch (MiniMC::Support::ConfigurationException& e) {
    messager.message<MiniMC::Support::Severity::Error> (e.what ());
    return static_cast<int>(MiniMC::Host::ExitCodes::ConfigurationError); 
  }
  catch (MiniMC::Support::Exception& e) {
    messager.message<MiniMC::Support::Severity::Error> (e.what ());
    return static_cast<int>(MiniMC::Host::ExitCodes::RuntimeError); 
  }
  
}
