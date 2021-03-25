#include <tuple>

#include "support/div.hpp"
#include "support/types.hpp"
#include "gtest/gtest.h"

class UDiv8  :public ::testing::TestWithParam<std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>> {  
};

TEST_P(UDiv8, KK) {
  MiniMC::uint8_t left = std::get<0> (GetParam());
  MiniMC::uint8_t right = std::get<1> (GetParam());
  MiniMC::uint8_t expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::div (left,right),expected);
}

INSTANTIATE_TEST_CASE_P(
        DivisionsFor8BitNumbers,
        UDiv8,
        ::testing::Values(
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(4,2,2),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(10,2,5),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(255,2,127),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(0,2,0)
			  
			  )
			);



class UDiv16  :public ::testing::TestWithParam<std::tuple<MiniMC::uint16_t,MiniMC::uint16_t,MiniMC::uint16_t>> {  
};

TEST_P(UDiv16, KK) {
  MiniMC::uint16_t left = std::get<0> (GetParam());
  MiniMC::uint16_t right = std::get<1> (GetParam());
  MiniMC::uint16_t expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::div (left,right),expected);
}


INSTANTIATE_TEST_CASE_P(
        DivisionsFor16BitNumbers,
        UDiv16,
        ::testing::Values(
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(4,2,2),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(10,2,5),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(512,2,256),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(0,2,0)
			  
			  )
			);


class UDiv32  :public ::testing::TestWithParam<std::tuple<MiniMC::uint32_t,MiniMC::uint32_t,MiniMC::uint32_t>> {  
};

TEST_P(UDiv32, KK) {
  MiniMC::uint32_t left = std::get<0> (GetParam());
  MiniMC::uint32_t right = std::get<1> (GetParam());
  MiniMC::uint32_t expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::div (left,right),expected);
}


INSTANTIATE_TEST_CASE_P(
        DivisionsFor32BitNumbers,
        UDiv32,
        ::testing::Values(
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(4,2,2),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(10,2,5),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(512,2,256),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(0,2,0)
			  
			  )
			);


class UDiv64  :public ::testing::TestWithParam<std::tuple<MiniMC::uint64_t,MiniMC::uint64_t,MiniMC::uint64_t>> {  
};

TEST_P(UDiv64, KK) {
  MiniMC::uint32_t left = std::get<0> (GetParam());
  MiniMC::uint32_t right = std::get<1> (GetParam());
  MiniMC::uint32_t expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::div (left,right),expected);
}


INSTANTIATE_TEST_CASE_P(
        DivisionsFor64BitNumbers,
        UDiv64,
        ::testing::Values(
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(4,2,2),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(10,2,5),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(512,2,256),
			  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(0,2,0)
			  
			  )
			);

