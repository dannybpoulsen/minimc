#include <tuple>

#include "support/rightshifts.hpp"
#include "support/types.hpp"
#include "gtest/gtest.h"

class LShr8  :public ::testing::TestWithParam<std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>> {  
};

TEST_P(LShr8, KK) {
  MiniMC::uint8_t left = std::get<0> (GetParam());
  MiniMC::uint8_t right = std::get<1> (GetParam());
  MiniMC::uint8_t expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::lshr (left,right),expected);
}

INSTANTIATE_TEST_CASE_P(
			LogicalShift8BitNumbers,
			LShr8,
			::testing::Values(
					  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(2,1,1)
					  )
			);



class AShr8  :public ::testing::TestWithParam<std::tuple<MiniMC::int8_t,MiniMC::int8_t,MiniMC::int8_t>> {  
};

TEST_P(AShr8, KK) {
  MiniMC::uint8_t left = MiniMC::bit_cast<MiniMC::int8_t, MiniMC::uint8_t>(std::get<0> (GetParam()));
  MiniMC::uint8_t right = MiniMC::bit_cast<MiniMC::int8_t, MiniMC::uint8_t>(std::get<1> (GetParam()));
  MiniMC::int8_t expected = std::get<2> (GetParam());
  auto res = MiniMC::bit_cast<MiniMC::uint8_t, MiniMC::int8_t>(MiniMC::Support::ashr (left,right)); 
  EXPECT_EQ(res,expected);
}

INSTANTIATE_TEST_CASE_P(
			ArithmeticShift8BitNumbers,
			AShr8,
			::testing::Values(
					  std::tuple<MiniMC::int8_t,MiniMC::int8_t,MiniMC::int8_t>(-2,1,-1)
					  )
			);


