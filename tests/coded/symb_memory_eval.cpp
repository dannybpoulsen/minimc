#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "minimc/model/variables.hpp"
#include "minimc/smt/smt.hpp"
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

TEST_CASE("Memory constants translate to SMT arrays") {
  using namespace MiniMC;

  auto backend = Support::SMT::SMTSolverRepository::get().getBackend("CVC4");
  REQUIRE(backend.has_value());
  auto context = backend->makeContext();
  Support::SMT::Translator translator{context};

  Model::MemoryConstantBuilder memoryBuilder;
  auto firstPointer = Model::pointer_t::makeHeapPointer(1, 0);
  Util::Array firstBytes{3};
  firstBytes.get_direct_access()[0] = 0x2a;
  firstBytes.get_direct_access()[1] = 0x7f;
  firstBytes.get_direct_access()[2] = 0x01;
  memoryBuilder.addEntry(firstPointer, firstBytes);

  auto secondPointer = Model::pointer_t::makeHeapPointer(2, 0);
  Util::Array secondBytes{1};
  secondBytes.get_direct_access()[0] = 0xa5;
  memoryBuilder.addEntry(secondPointer, secondBytes);
  auto memoryConstant = memoryBuilder.build();
  auto memory = translator.Translate(*memoryConstant);

  Model::MemoryConstantBuilder emptyBuilder;
  auto emptyMemoryConstant = emptyBuilder.build();
  auto emptyMemory = translator.Translate(*emptyMemoryConstant);
  REQUIRE(emptyMemory != nullptr);

  auto& solver = context->getSolver();
  auto& builder = context->getBuilder();
  Model::Pointer nullPointer{Model::pointer_t::makeNullPointer()};
  auto emptyByte = builder.buildTerm(
      SMTLib::Ops::Select, {emptyMemory, translator(nullPointer)});
  solver.assert_formula(builder.buildTerm(SMTLib::Ops::Equal, {emptyByte, emptyByte}));

  auto assertBytes = [&](const Model::pointer_t& pointer, std::span<const BV8> bytes) {
    Model::Pointer pointerValue{pointer};
    auto baseAddress = translator(pointerValue);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
      auto offset = builder.makeBVIntConst(i, 64);
      auto address = builder.buildTerm(SMTLib::Ops::BVAdd, {baseAddress, offset});
      auto actual = builder.buildTerm(SMTLib::Ops::Select, {memory, address});
      auto expected = builder.makeBVIntConst(bytes[i], 8);
      solver.assert_formula(builder.buildTerm(SMTLib::Ops::Equal, {actual, expected}));
    }
  };

  assertBytes(firstPointer, firstBytes.get_direct_access());
  assertBytes(secondPointer, secondBytes.get_direct_access());
  CHECK(solver.check_sat() == SMTLib::Result::Satis);
}
