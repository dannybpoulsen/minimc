#include <boost/program_options.hpp>
#include "model/output.hpp"
#include "host/host.hpp"


#include "plugin.hpp"

namespace po = boost::program_options;

namespace {

  void addOptions (po::options_description&) {  }  
}
  
MiniMC::Host::ExitCodes convert_main (MiniMC::Model::Program&& prgm, const MiniMC::CPA::AnalysisBuilder&, MiniMC::Support::Messager& )  {
  MiniMC::Model::writeProgram (std::cout,prgm);
  
  return MiniMC::Host::ExitCodes::AllGood;
}

static CommandRegistrar enum_reg ("convert",convert_main,"output input program as MiniMC",addOptions);
  
