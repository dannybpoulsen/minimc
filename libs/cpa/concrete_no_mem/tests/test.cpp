#include <iostream>

#include "support/types.hpp"
#include "register.hpp"
#include "instructionimpl.hpp"

int main () {
  MiniMC::int64_t left = -10;
  MiniMC::int64_t right = 2;

  MiniMC::CPA::ConcreteNoMem::InRegister rl (&left,sizeof(MiniMC::int64_t));
  MiniMC::CPA::ConcreteNoMem::InRegister rr (&right,sizeof(MiniMC::int64_t));
  auto res = MiniMC::CPA::ConcreteNoMem::TACExec<MiniMC::Model::InstructionCode::SDiv,MiniMC::uint64_t>::execute (rl,rr);

  std::cout << res.template get<MiniMC::int64_t> () << std::endl;
}
