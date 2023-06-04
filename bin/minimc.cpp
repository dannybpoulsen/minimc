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
  if (options.unrollLoops) {
    controller.unrollLoops (options.unrollLoops);
  }
  if (options.inlineFunctions) {
    controller.inlineFunctions (options.inlineFunctions);
  }
}

int main(int argc, char* argv[]) {
  
  std::string input;
  std::string subcommand;
  MiniMC::Support::Messager messager;
  try {
    
    SetupOptions options;
    bool ok = parseOptions(argc, argv, options);
    
    if (ok) {
      // Load Program
      MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64>();
      MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64>(tfac);
      auto loader = options.load.registrar->makeLoader  (tfac,cfac);
      MiniMC::Model::Program prgm = loader->loadFromFile (options.load.inputname,messager);
      MiniMC::Model::Controller control(std::move(prgm));
      
      if (!control.typecheck (messager)) {
	return -1;
      }
      transformProgram (control,options.transform);
      
      if (options.outputname != "") {
	std::ofstream stream;
	stream.open (options.outputname, std::ofstream::out);
	MiniMC::Model::writeProgram (stream,control.getProgram ());
	stream.close ();
      }
      if (options.command) {
	auto res =  static_cast<int>(options.command->getFunction()(control,options.cpa,messager));
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
