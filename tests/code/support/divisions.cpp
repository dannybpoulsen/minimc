#include <tuple>

#include "support/operataions.hpp"
#include "host/types.hpp"
#include "gtest/gtest.h"

class UDiv8  :public ::testing::TestWithParam<std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>> {  
};

TEST_P(UDiv8, KK) {
  MiniMC::BV8 left = std::get<0> (GetParam());
  MiniMC::BV8 right = std::get<1> (GetParam());
  MiniMC::BV8 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::Op<MiniMC::Support::TAC::UDiv> (left,right),expected);
}

INSTANTIATE_TEST_CASE_P(
        DivisionsFor8BitNumbers,
        UDiv8,
        ::testing::Values(
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(4,2,2),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(10,2,5),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(255,2,127),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(0,2,0)
			  
			  )
			);



class UDiv16  :public ::testing::TestWithParam<std::tuple<MiniMC::BV16,MiniMC::BV16,MiniMC::BV16>> {  
};

TEST_P(UDiv16, KK) {
  MiniMC::BV16 left = std::get<0> (GetParam());
  MiniMC::BV16 right = std::get<1> (GetParam());
  MiniMC::BV16 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::Op<MiniMC::Support::TAC::UDiv> (left,right),expected);
}


INSTANTIATE_TEST_CASE_P(
        DivisionsFor16BitNumbers,
        UDiv16,
        ::testing::Values(
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(4,2,2),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(10,2,5),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(512,2,256),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(0,2,0)
			  
			  )
			);


class UDiv32  :public ::testing::TestWithParam<std::tuple<MiniMC::uint32_t,MiniMC::uint32_t,MiniMC::uint32_t>> {  
};

TEST_P(UDiv32, KK) {
  MiniMC::BV32 left = std::get<0> (GetParam());
  MiniMC::BV32 right = std::get<1> (GetParam());
  MiniMC::BV32 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::Op<MiniMC::Support::TAC::UDiv> (left,right),expected);
}


INSTANTIATE_TEST_CASE_P(
        DivisionsFor32BitNumbers,
        UDiv32,
        ::testing::Values(
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(4,2,2),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(10,2,5),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(512,2,256),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(0,2,0)
			  
			  )
			);


class UDiv64  :public ::testing::TestWithParam<std::tuple<MiniMC::uint64_t,MiniMC::uint64_t,MiniMC::uint64_t>> {  
};

TEST_P(UDiv64, KK) {
  MiniMC::BV32 left = std::get<0> (GetParam());
  MiniMC::BV32 right = std::get<1> (GetParam());
  MiniMC::BV32 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::Op<MiniMC::Support::TAC::UDiv> (left,right),expected);
}


INSTANTIATE_TEST_CASE_P(
        DivisionsFor64BitNumbers,
        UDiv64,
        ::testing::Values(
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(4,2,2),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(10,2,5),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(512,2,256),
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(0,2,0)
			  
			  )
			);

