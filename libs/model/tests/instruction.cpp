#include <iostream>

#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "model/variables.hpp"

int main () {
  MiniMC::Model::Variable_ptr res = std::make_shared<MiniMC::Model::Variable> ("Res");
  MiniMC::Model::Variable_ptr v1 = std::make_shared<MiniMC::Model::Variable> ("var1");
  MiniMC::Model::Variable_ptr v2 = std::make_shared<MiniMC::Model::Variable> ("var2");
  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Sub> builder;
  builder.setRes (res);
  builder.setLeft (v1);
  builder.setRight (v2);
  MiniMC::Model::Instruction inst = builder.BuildInstruction ();
  auto helper = MiniMC::Model::makeHelper<MiniMC::Model::InstructionCode::Sub> (inst);
  std::cout << inst << std::endl;
}
