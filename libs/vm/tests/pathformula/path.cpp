#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"
#include "smt/smt.hpp"
#include "vm/pathformula/pathformua.hpp"
#include "vm/pathformula/operations.hpp"


#include <model/variables.hpp>
#include <vector>
#include <iostream>

TEST_CASE ("Check Pointer interpretation") {
  std::vector<MiniMC::Support::SMT::SMTDescr> descrs;
  MiniMC::Support::SMT::getSMTBackends (std::back_inserter(descrs));
  
  REQUIRE (descrs.size () > 0);

  auto context = descrs.at(0).makeContext ();
  MiniMC::VMT::Pathformula::ValueLookup vlookup{0,context->getBuilder ()};
  auto input = MiniMC::pointer_t{1,2,3};
  MiniMC::Model::Value_ptr ptr = std::make_shared<MiniMC::Model::Pointer> (input);

  auto p_ptr = vlookup.lookupValue(ptr);
  auto solver = context->makeSolver ();
  REQUIRE (solver->check_sat () == SMTLib::Result::Satis);
  REQUIRE (p_ptr.is<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer> ());

  auto res = p_ptr.as<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer> ().interpretValue (*solver);
  
  CHECK(res.offset == input.offset);
  CHECK(res.base == input.base);
  CHECK(res.segment == input.segment);
}


TEST_CASE ("Check Pointer32 interpretation") {
  std::vector<MiniMC::Support::SMT::SMTDescr> descrs;
  MiniMC::Support::SMT::getSMTBackends (std::back_inserter(descrs));
  
  REQUIRE (descrs.size () > 0);

  auto context = descrs.at(0).makeContext ();
  MiniMC::VMT::Pathformula::ValueLookup vlookup{0,context->getBuilder ()};
  auto input = MiniMC::pointer32_t{1,2,3};
  MiniMC::Model::Value_ptr ptr = std::make_shared<MiniMC::Model::Pointer32> (input);

  auto p_ptr = vlookup.lookupValue(ptr);
  auto solver = context->makeSolver ();
  REQUIRE (solver->check_sat () == SMTLib::Result::Satis);
  REQUIRE (p_ptr.is<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer32> ());

  auto res = p_ptr.as<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer32> ().interpretValue (*solver);
  
  CHECK(res.offset == input.offset);
  CHECK(res.base == input.base);
  CHECK(res.segment == input.segment);
}

TEST_CASE ("Pointer to Pointer32") {
  std::vector<MiniMC::Support::SMT::SMTDescr> descrs;
  MiniMC::Support::SMT::getSMTBackends (std::back_inserter(descrs));
  
  REQUIRE (descrs.size () > 0);

  auto context = descrs.at(0).makeContext ();
  MiniMC::VMT::Pathformula::ValueLookup vlookup{0,context->getBuilder ()};
  MiniMC::VMT::Pathformula::Casts<MiniMC::VMT::Pathformula::PathFormulaVMVal> caster{context->getBuilder ()};
  
  auto input = MiniMC::pointer_t{1,2,3};
  MiniMC::Model::Value_ptr ptr = std::make_shared<MiniMC::Model::Pointer> (input);
  
  auto p_ptr = vlookup.lookupValue(ptr);
  REQUIRE (p_ptr.is<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer> ());
    
  auto casted = caster.PtrToPtr32 (p_ptr.as<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer> ());
  auto solver = context->makeSolver ();
  REQUIRE (solver->check_sat () == SMTLib::Result::Satis);

  auto res = casted.interpretValue (*solver);
  
  CHECK(res.offset == input.offset);
  CHECK(res.base == input.base);
  CHECK(res.segment == input.segment);
}


TEST_CASE ("Pointer32 to Pointer") {
  std::vector<MiniMC::Support::SMT::SMTDescr> descrs;
  MiniMC::Support::SMT::getSMTBackends (std::back_inserter(descrs));
  
  REQUIRE (descrs.size () > 0);

  auto context = descrs.at(0).makeContext ();
  MiniMC::VMT::Pathformula::ValueLookup vlookup{0,context->getBuilder ()};
  MiniMC::VMT::Pathformula::Casts<MiniMC::VMT::Pathformula::PathFormulaVMVal> caster{context->getBuilder ()};
  
  auto input = MiniMC::pointer32_t{1,2,3};
  MiniMC::Model::Value_ptr ptr = std::make_shared<MiniMC::Model::Pointer32> (input);
  
  auto p_ptr = vlookup.lookupValue(ptr);
  REQUIRE (p_ptr.is<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer32> ());
    
  auto casted = caster.Ptr32ToPtr (p_ptr.as<MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer32> ());
  auto solver = context->makeSolver ();
  REQUIRE (solver->check_sat () == SMTLib::Result::Satis);

  auto res = casted.interpretValue (*solver);
  
  CHECK(res.offset == input.offset);
  CHECK(res.base == input.base);
  CHECK(res.segment == input.segment);
}
