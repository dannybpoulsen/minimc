#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"


#include <minimc/model/symbol.hpp>
#include <minimc/model/registers.hpp>
#include <minimc/model/types.hpp>




TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto smb = frame.makeSymbol ("main");
  CHECK (smb.getName () == "main");
  CHECK (smb.getFullName () == ":main");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  CHECK (smb.getName () == "echo");
  CHECK (smb.getFullName () == ":main:echo");
}


TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  sframe.close ();
  CHECK (smb.getName () == "echo");
  CHECK (smb.getFullName () == ":main:echo");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("main");
  std::optional<MiniMC::Model::Symbol> symb;
  symb = frame.resolve ("main");
  REQUIRE (symb);
  CHECK (symb.value().getFullName () == ":main");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  std::optional<MiniMC::Model::Symbol> symb = sframe.resolve ("echo");
  
  REQUIRE (symb  );
  CHECK (symb.value().getName () == "echo");
  CHECK (symb.value().getFullName () == ":main:echo");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  std::optional<MiniMC::Model::Symbol> symb = sframe.resolve ("delta");
  

  REQUIRE (symb );
  CHECK (symb.value().getName () == "delta");
  CHECK (symb.value().getFullName () == ":delta");
}


TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");

  
  std::optional<MiniMC::Model::Symbol> symb = sframe.resolve ("echo");
  
  REQUIRE (symb );
  
  CHECK (symb.value().getName () == "echo");
  CHECK (symb.value().getFullName () == ":main:echo");
}

TEST_CASE("Frame Resolve") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");

  
  auto  symb =sframe.resolveQualified ("main:echo") ;
  REQUIRE (symb);
  
  CHECK (symb.value().getName () == "echo");
  CHECK (symb.value().getFullName () == ":main:echo");
}

TEST_CASE("Frame Resolve") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  
  
  auto symb = sframe.resolveQualified ("delta");
  REQUIRE (symb);
  
  CHECK (symb.value().getName () == "delta");
  CHECK (symb.value().getFullName () == ":delta");
}

TEST_CASE("Frame Resolve Root") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  
  auto  symb =  frame.resolveQualified ("delta");
  REQUIRE (symb);
  
  CHECK (symb.value().getName () == "delta");
  CHECK (symb.value().getFullName () == ":delta");
}

TEST_CASE("Frame owns registers by category") {
  MiniMC::Model::Frame frame{"registers"};
  auto copy = frame;
  auto type = MiniMC::Model::I8Type::get();

  auto local = frame.makeRegister("local", type);
  auto cpu = frame.makeRegister("cpu", type, MiniMC::Model::RegType::CPU);
  auto second_local = copy.makeFreshRegister(type, MiniMC::Model::RegType::Local);
  auto second_cpu = copy.makeRegister("cpu2", type, MiniMC::Model::RegType::CPU);
  frame.makeSymbol("location");

  CHECK(local->getId() == 0);
  CHECK(cpu->getId() == 0);
  CHECK(second_local->getId() == 1);
  CHECK(second_cpu->getId() == 1);
  CHECK(frame.numberOfRegisters() == 4);
  CHECK(frame.numberOfRegisters(MiniMC::Model::RegType::Local) == 2);
  CHECK(frame.numberOfRegisters(MiniMC::Model::RegType::CPU) == 2);

  std::size_t local_count = 0;
  for (auto [_, reg] : frame.local_registers(MiniMC::Model::RegType::Local)) {
    CHECK(reg->getRegType() == MiniMC::Model::RegType::Local);
    ++local_count;
  }
  CHECK(local_count == 2);

  auto symbol = frame.resolve("local");
  REQUIRE(symbol);
  CHECK(std::get<MiniMC::Model::Register_ptr>(symbol->getUserData()) == local);
  CHECK_THROWS(frame.makeRegister("local", type, MiniMC::Model::RegType::Local));
}

TEST_CASE("Frame register IDs are shared across frame copies") {
  MiniMC::Model::Frame frame{"factory"};
  auto frame_copy = frame;
  auto type = MiniMC::Model::I8Type::get();

  auto first = frame.makeRegister("first", type, MiniMC::Model::RegType::Local);
  auto second = frame_copy.makeFreshRegister(type, MiniMC::Model::RegType::Local);

  CHECK(first->getId() == 0);
  CHECK(second->getId() == 1);
  CHECK(frame.numberOfRegisters(MiniMC::Model::RegType::Local) == 2);
}

TEST_CASE("Only root frames can contain CPU and persistent registers") {
  MiniMC::Model::Frame root{"program"};
  auto child = root.create("function");
  auto type = MiniMC::Model::I8Type::get();

  CHECK_NOTHROW(root.makeRegister("cpu", type, MiniMC::Model::RegType::CPU));
  CHECK_NOTHROW(root.makeFreshRegister(type, MiniMC::Model::RegType::Meta));
  CHECK_NOTHROW(root.makeRegister("persistent", type, MiniMC::Model::RegType::Persistent));
  CHECK_NOTHROW(child.makeRegister("local", type));
  auto meta = child.makeFreshRegister(type, MiniMC::Model::RegType::Meta);
  CHECK(meta->getId() == 0);

  const auto symbols_before = std::ranges::distance(child.local_symbols());
  CHECK_THROWS(child.makeRegister("cpu", type, MiniMC::Model::RegType::CPU));
  CHECK_THROWS(child.makeFreshRegister(type, MiniMC::Model::RegType::Persistent));
  CHECK(std::ranges::distance(child.local_symbols()) == symbols_before);
  CHECK(child.numberOfRegisters() == 2);
  CHECK(child.numberOfRegisters(MiniMC::Model::RegType::Meta) == 1);
  CHECK_NOTHROW(child.makeFreshRegister(type, MiniMC::Model::RegType::Meta));
  CHECK(child.numberOfRegisters(MiniMC::Model::RegType::Meta) == 2);
}
