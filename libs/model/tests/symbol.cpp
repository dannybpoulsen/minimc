#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"


#include <model/symbol.hpp>




TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto smb = frame.makeSymbol ("main");
  CHECK (smb.getName () == "main");
  CHECK (smb.getFullName () == "main");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  CHECK (smb.getName () == "echo");
  CHECK (smb.getFullName () == "main:echo");
}


TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  sframe.close ();
  CHECK (smb.getName () == "echo");
  CHECK (smb.getFullName () == "main:echo");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("main");
  MiniMC::Model::Symbol symb;
  
  REQUIRE (frame.resolve ("main",symb));
  CHECK (symb.getFullName () == "main");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");

  MiniMC::Model::Symbol symb;

  REQUIRE (sframe.resolve ("echo",symb));
  CHECK (symb.getName () == "echo");
  CHECK (symb.getFullName () == "main:echo");
}

TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");

  MiniMC::Model::Symbol symb;

  REQUIRE (sframe.resolve ("delta",symb));
  CHECK (symb.getName () == "delta");
  CHECK (symb.getFullName () == "delta");
}


TEST_CASE("Frame") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");

  
  MiniMC::Model::Symbol symb;
  REQUIRE (sframe.resolve ("echo",symb));
  
  CHECK (symb.getName () == "echo");
  CHECK (symb.getFullName () == "main:echo");
}

TEST_CASE("Frame Resolve") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");

  
  MiniMC::Model::Symbol symb;
  REQUIRE (sframe.resolveQualified ("main:echo",symb));
  
  CHECK (symb.getName () == "echo");
  CHECK (symb.getFullName () == "main:echo");
}

TEST_CASE("Frame Resolve") {
  MiniMC::Model::Frame frame;
  frame.makeSymbol ("delta");
  auto sframe  = frame.create ("main");
  
  auto smb = sframe.makeSymbol ("echo");
  
  
  MiniMC::Model::Symbol symb;
  REQUIRE (sframe.resolveQualified ("delta",symb));
  
  CHECK (symb.getName () == "delta");
  CHECK (symb.getFullName () == "delta");
}



