#include "minimc/model/output.hpp"
#include "plugin.hpp"
#include "minimc/support/localisation.hpp"
#include "minimc/model/checkers/typechecker.hpp"
#include "minimc/model/checkers/structuralchecker.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include <boost/program_options.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>


#include "minimc/loaders/loader.hpp"
#include "options.hpp"

namespace po = boost::program_options;

MiniMC::Model::Program transformProgram (MiniMC::Model::Program&& prgm, const transform_options& options, MiniMC::Support::Messager& mess) {
  MiniMC::Model::Modifications::ProgramManager manager;
  using namespace  MiniMC::Model::Modifications;
  
  manager.add<SplitAsserts> ();
  if (options.unrollLoops) {
    manager.add<UnrollLoops> (options.unrollLoops);
  }
  if (options.expand_nondet) {
    manager.add<NonDetExpander> (mess);
  }
  if (options.inlineFunctions) {
    manager.add<InlineFunctions> (options.inlineFunctions);
  }
  if (options.lower_phi) {
    manager.add<LowerPhi> ();
  }
  return manager (std::move(prgm));
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
      auto loader = options.load.loader;
      MiniMC::Model::Program prgm = loader->loadFromFile (options.load.inputname,tfac,cfac,messager);
      
      if (!MiniMC::Model::Checkers::TypeChecker{prgm}.Check (messager) ||
	  !MiniMC::Model::Checkers::StructuralChecker{}.Check (prgm,messager)
	  ) {
	return -1;
      }
      MiniMC::Model::Program prgm2 = transformProgram (std::move(prgm),options.transform, messager);
      
      if (options.outputname != "") {
	std::ofstream stream;
	stream.open (options.outputname, std::ofstream::out);
	MiniMC::Model::writeProgram (stream,prgm2);
	stream.close ();
      }
      if (options.command) {
	auto res =  static_cast<int>(options.command->runCommand(std::move(prgm2),messager,options));
	return res;
      }
      
      else {
	messager << MiniMC::Support::TError<std::string> {"No subcommand selected"};
	return static_cast<int>(MiniMC::Host::ExitCodes::ConfigurationError);
      }
    }
  }
  catch (MiniMC::Support::ConfigurationException& e) {
    messager << MiniMC::Support::TError { e.what ()};
    return static_cast<int>(MiniMC::Host::ExitCodes::ConfigurationError); 
  }
  catch (MiniMC::Support::Exception& e) {
    messager << MiniMC::Support::TError {e.what ()};
    return static_cast<int>(MiniMC::Host::ExitCodes::RuntimeError); 
  }
  
}
