#include "minimc/model/VIL.hpp"
#include "minimc/model/output.hpp"

#include "minimc/io/ostream.hpp"

int main () {
 
  MiniMC::Model::VIL::StatementBuilder builder;
  MiniMC::Model::VIL::VILtoCFA converter;
  
  builder.I32(10);
  builder.Ret ();
  builder.I32(20);
  builder.Ret ();
  builder.I32(20);
  builder.I32(30);
  builder.Eq ();
  builder.If();

  auto s = builder.getStatement();
  
  s->output(MiniMC::IO::os_ostream::out());
    
  MiniMC::Model::writeCFA (std::cout,converter.convert (*s,MiniMC::Model::Frame{}));
}
