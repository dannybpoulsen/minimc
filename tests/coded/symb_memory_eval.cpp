#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "minimc/model/variables.hpp"
#include "minimc/values/symb/symb.hpp"

TEST_CASE("Symbolic memory evaluates to a memory constant") {
  using namespace MiniMC;
  using namespace MiniMC::Values::Symb;

  ConstraintSolver solver;

  Value::Memory memory{Model::Undef::make(Model::MemoryType::get())};
  auto pointer = Model::pointer_t::makeHeapPointer(1, 0);
  Value::Pointer valuePointer{Model::Pointer::make(pointer)};
  Value::I64 size{Model::I64Integer::make(2)};

  MemoryOps memops;
  memory = memops.allocate(memory, valuePointer, size);
  memory = memops.store(memory, valuePointer, Value::I8{Model::I8Integer::make(0x2a)});
  REQUIRE(solver.check() == MiniMC::VMT::Feasibility::Feasible);

  Value::Memory emptyMemory{Model::Undef::make(Model::MemoryType::get())};
  auto emptyConstant = std::dynamic_pointer_cast<Model::MemoryConstant>(solver.eval(Value{emptyMemory}));
  REQUIRE(emptyConstant != nullptr);
  CHECK(emptyConstant->getEntries().empty());

  auto constant = solver.eval(Value{memory});
  auto memoryConstant = std::dynamic_pointer_cast<Model::MemoryConstant>(constant);
  REQUIRE(memoryConstant != nullptr);
  REQUIRE(memoryConstant->getEntries().size() == 1);

  const auto& entry = memoryConstant->getEntries().front();
  CHECK(entry.baseobj == Model::pointer_t::makeHeapPointer(1, 0));
  REQUIRE(entry.arr.getSize() == 2);
  CHECK(entry.arr.get_direct_access()[0] == 0x2a);
}
