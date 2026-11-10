#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"


#include <minimc/model/symbol.hpp>
#include <minimc/model/registers.hpp>
#include <minimc/model/types.hpp>

#include <ranges>



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

TEST_CASE("Symbol predicates classify user data kinds") {
  using namespace MiniMC::Model;

  Frame frame{"predicates"};
  auto register_symbol = frame.makeSymbol("register");
  register_symbol.setUserData(Register_ptr{});
  auto function_symbol = frame.makeSymbol("function");
  function_symbol.setUserData(Function_ptr{});
  auto location_symbol = frame.makeSymbol("location");
  location_symbol.setUserData(Location_wptr{});
  auto heap_block_symbol = frame.makeSymbol("heap_block");
  heap_block_symbol.setUserData(HeapBlock_wptr{});
  auto untyped_symbol = frame.makeSymbol("untyped");

  CHECK(SymbolPredicates::is_register(register_symbol));
  CHECK_FALSE(SymbolPredicates::is_function(register_symbol));
  CHECK(SymbolPredicates::is_function(function_symbol));
  CHECK(SymbolPredicates::is_location(location_symbol));
  CHECK(SymbolPredicates::is_heap_block(heap_block_symbol));
  CHECK(SymbolPredicates::is_untyped(untyped_symbol));
  CHECK_FALSE(SymbolPredicates::is_register(untyped_symbol));
}

TEST_CASE("Register type predicates filter symbol ranges") {
  using namespace MiniMC::Model;

  Frame root{"registers"};
  auto type = I8Type::get();
  root.makeRegister("cpu", type, RegType::CPU);
  root.makeRegister("persistent", type, RegType::Persistent);
  auto child = root.create("function");
  child.makeRegister("local", type, RegType::Local);
  child.makeRegister("meta", type, RegType::Meta);
  auto empty_register = child.makeSymbol("empty_register");
  empty_register.setUserData(Register_ptr{});
  auto untyped_symbol = child.makeSymbol("untyped");

  std::size_t local_register_count = 0;
  for (const auto& symbol : child.local_symbols() | std::views::filter(SymbolPredicates::is_register)) {
    CHECK(SymbolPredicates::is_register(symbol));
    ++local_register_count;
  }
  CHECK(local_register_count == 3);
  CHECK(child.numberOfRegisters() == 3);

  std::size_t local_count = 0;
  for (const auto& symbol : child.local_symbols()
       | std::views::filter(SymbolPredicates::is_register_of_type(RegType::Local))) {
    CHECK(SymbolPredicates::is_register(symbol));
    ++local_count;
  }
  CHECK(local_count == 1);
  CHECK(child.numberOfRegisters(RegType::Local) == 1);
  CHECK(child.numberOfRegisters(RegType::Meta) == 1);
  CHECK(child.numberOfRegisters(RegType::CPU) == 0);

  std::size_t meta_count = 0;
  for (const auto& symbol : child.local_and_parent_symbols()
       | std::views::filter(SymbolPredicates::is_register_of_type(RegType::Meta))) {
    CHECK(SymbolPredicates::is_register(symbol));
    ++meta_count;
  }
  CHECK(meta_count == 1);

  std::size_t cpu_count = 0;
  for (const auto& symbol : child.local_and_parent_symbols()
       | std::views::filter(SymbolPredicates::is_register_of_type(RegType::CPU))) {
    CHECK(SymbolPredicates::is_register(symbol));
    ++cpu_count;
  }
  CHECK(cpu_count == 1);

  std::size_t persistent_count = 0;
  for (const auto& symbol : child.local_and_parent_symbols()
       | std::views::filter(SymbolPredicates::is_register_of_type(RegType::Persistent))) {
    CHECK(SymbolPredicates::is_register(symbol));
    ++persistent_count;
  }
  CHECK(persistent_count == 1);

  CHECK_FALSE(SymbolPredicates::is_register_of_type(RegType::Local)(empty_register));
  CHECK_FALSE(SymbolPredicates::is_register_of_type(RegType::CPU)(empty_register));
  CHECK_FALSE(SymbolPredicates::is_register_of_type(RegType::Meta)(untyped_symbol));
}
