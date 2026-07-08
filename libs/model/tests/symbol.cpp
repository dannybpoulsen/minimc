#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"


#include <minimc/model/symbol.hpp>




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


