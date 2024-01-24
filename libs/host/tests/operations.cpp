
#include <sstream>
#include "doctest/doctest.h"
#include "minimc/host/types.hpp"
#include "minimc/host/operataions.hpp"


TEST_SUITE ("Addition") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV8{0},MiniMC::BV8{0}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV8{0},MiniMC::BV8{1}) == MiniMC::BV8(1));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV8{255},MiniMC::BV8{1}) == MiniMC::BV8(0));
  }
  TEST_CASE ("BV16") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV16{0},MiniMC::BV16{0}) == MiniMC::BV16(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV16{0},MiniMC::BV16{1}) == MiniMC::BV16(1));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV16{255},MiniMC::BV16{1}) == MiniMC::BV16(256));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV16{0xFFFF},MiniMC::BV16{1}) == MiniMC::BV16(0));
    
  }
  TEST_CASE ("BV32") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV32{0},MiniMC::BV32{0}) == MiniMC::BV32(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV32{0},MiniMC::BV32{1}) == MiniMC::BV32(1));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV32{255},MiniMC::BV32{1}) == MiniMC::BV32(256));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV32{0xFFFF},MiniMC::BV32{1}) == MiniMC::BV32(0x10000));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV32{0xFFFFFFFF},MiniMC::BV32{1}) == MiniMC::BV32(0)); 
  }
  TEST_CASE ("BV64") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV64{0},MiniMC::BV64{0}) == MiniMC::BV64(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV64{0},MiniMC::BV64{1}) == MiniMC::BV64(1));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV64{255},MiniMC::BV64{1}) == MiniMC::BV64(256));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV64{0xFFFF},MiniMC::BV64{1}) == MiniMC::BV64(0x10000));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV64{0xFFFFFFFF},MiniMC::BV64{1}) == MiniMC::BV64(0x100000000)); 
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Add> (MiniMC::BV64{0xFFFFFFFFFFFFFFFF},MiniMC::BV64{1}) == MiniMC::BV64(0));
  }
}


TEST_SUITE ("Subtraction") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV8{0},MiniMC::BV8{0}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV8{10},MiniMC::BV8{2}) == MiniMC::BV8(8));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV8{0},MiniMC::BV8{1}) == MiniMC::BV8(255));
  }

  TEST_CASE ("B16") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV16{0},MiniMC::BV16{0}) == MiniMC::BV16(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV16{10},MiniMC::BV16{2}) == MiniMC::BV16(8));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV16{0},MiniMC::BV16{1}) == MiniMC::BV16(0xFFFF));
  }

  TEST_CASE ("BV32") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV32{0},MiniMC::BV32{0}) == MiniMC::BV32(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV32{10},MiniMC::BV32{2}) == MiniMC::BV32(8));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV32{0},MiniMC::BV32{1}) == MiniMC::BV32(0xFFFFFFFF));
  }

  TEST_CASE ("BV64") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV64{0},MiniMC::BV64{0}) == MiniMC::BV64(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV64{10},MiniMC::BV64{2}) == MiniMC::BV64(8));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Sub> (MiniMC::BV64{0},MiniMC::BV64{1}) == MiniMC::BV64(0xFFFFFFFFFFFFFFFF));
  }
  
}

TEST_SUITE ("Unsigned Division") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::UDiv> (MiniMC::BV8{0},MiniMC::BV8{1}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::UDiv> (MiniMC::BV8{10},MiniMC::BV8{2}) == MiniMC::BV8(5));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::UDiv> (MiniMC::BV8{0},MiniMC::BV8{1}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::UDiv> (MiniMC::BV8{11},MiniMC::BV8{3}) == MiniMC::BV8(3));
  }
}

TEST_SUITE ("Signed Division") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::BV8{0},MiniMC::BV8{1}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::BV8{10},MiniMC::BV8{2}) == MiniMC::BV8(5));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::BV8{0},MiniMC::BV8{1}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::BV8{11},MiniMC::BV8{3}) == MiniMC::BV8(3));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::FromHostSigned<8>(-1),MiniMC::BV8{1}) == MiniMC::FromHostSigned<8> (-1));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::FromHostSigned<8>(-1),MiniMC::BV8{1}) == MiniMC::FromHostSigned<8> (-1));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::FromHostSigned<8>(-127),MiniMC::BV8{2}) == MiniMC::FromHostSigned<8> (-63));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::SDiv> (MiniMC::FromHostSigned<8>(-127),MiniMC::BV8{2}) == MiniMC::FromHostSigned<8> (-63));
  }
}

TEST_SUITE ("Multiplication") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Mul> (MiniMC::BV8{0},MiniMC::BV8{1}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Mul> (MiniMC::BV8{10},MiniMC::BV8{2}) == MiniMC::BV8(20));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Mul> (MiniMC::FromHostSigned<8>(-1),MiniMC::BV8{2}) == MiniMC::FromHostSigned<8> (-2));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Mul> (MiniMC::FromHostSigned<8>(-1),MiniMC::FromHostSigned<8>(-127)) == MiniMC::FromHostSigned<8> (127));
  }
}

TEST_SUITE ("Shift left") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Shl> (MiniMC::BV8{1},MiniMC::BV8{1}) == MiniMC::BV8(2));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Shl> (MiniMC::BV8{255},MiniMC::BV8{8}) == MiniMC::BV8(0));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::Shl> (MiniMC::BV8{255},MiniMC::BV8{7}) == MiniMC::BV8(128));
    
  }
}


TEST_SUITE ("Arithmetic Shift Right") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV8{255},MiniMC::BV8{1}) == MiniMC::BV8(255));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV8{127},MiniMC::BV8{1}) == MiniMC::BV8(63));    
  }
  TEST_CASE ("BV16") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV16{0xFF},MiniMC::BV16{1}) == MiniMC::BV16(0x7F));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV16{0xFFFF},MiniMC::BV16{1}) == MiniMC::BV16(0xFFFF));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV16{127},MiniMC::BV16{1}) == MiniMC::BV16(63));    
  }

  TEST_CASE ("BV32") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV32{0xFF},MiniMC::BV32{1}) == MiniMC::BV32(0x7F));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV32{0xFFFF},MiniMC::BV32{1}) == MiniMC::BV32(0x7FFF));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::AShr> (MiniMC::BV32{0xFFFFFFFF},MiniMC::BV32{1}) == MiniMC::BV32(0xFFFFFFFF));
  }
}

TEST_SUITE ("Logical Shift Right") {
  TEST_CASE ("BV8") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV8{255},MiniMC::BV8{1}) == MiniMC::BV8(127));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV8{127},MiniMC::BV8{1}) == MiniMC::BV8(63));
    
  }

  TEST_CASE ("BV16") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV16{255},MiniMC::BV16{1}) == MiniMC::BV16(127));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV16{127},MiniMC::BV16{1}) == MiniMC::BV16(63));
    
  }

  TEST_CASE ("BV32") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV32{255},MiniMC::BV32{1}) == MiniMC::BV32(127));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV32{127},MiniMC::BV32{1}) == MiniMC::BV32(63));
    
  }

  TEST_CASE ("BV64") {
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV64{255},MiniMC::BV64{1}) == MiniMC::BV64(127));
    CHECK (MiniMC::Host::Op<MiniMC::Host::TAC::LShr> (MiniMC::BV64{127},MiniMC::BV64{1}) == MiniMC::BV64(63));
    
  }
}

