#include <tuple>

#include "support/operataions.hpp"
#include "host/types.hpp"
#include "gtest/gtest.h"

class SDiv8  :public ::testing::TestWithParam<std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>> {  
};

TEST_P(SDiv8, KK) {
  MiniMC::BV8 left = MiniMC::bit_cast<MiniMC::int8_t,MiniMC::BV8> (std::get<0> (GetParam()));
  MiniMC::BV8 right = MiniMC::bit_cast<MiniMC::int8_t,MiniMC::BV8> (std::get<1> (GetParam()));
  MiniMC::int8_t expected = std::get<2> (GetParam());
  auto res =MiniMC::bit_cast<MiniMC::BV8,MiniMC::int8_t> (MiniMC::Support::Op<MiniMC::Support::TAC::SDiv> (left,right)); 
  EXPECT_EQ(res,expected);
}

INSTANTIATE_TEST_CASE_P(
        SDivisionsFor8BitNumbers,
        SDiv8,
        ::testing::Values(
			  std::tuple<MiniMC::int8_t,MiniMC::int8_t,MiniMC::int8_t>(-4,2,-2)
			  
			  )
			);



class SDiv16  :public ::testing::TestWithParam<std::tuple<MiniMC::int16_t,MiniMC::int16_t,MiniMC::int16_t>> {  
};

TEST_P(SDiv16, KK) {
  MiniMC::BV16 left = MiniMC::bit_cast<MiniMC::int16_t,MiniMC::uint16_t> (std::get<0> (GetParam()));
  MiniMC::BV16 right = MiniMC::bit_cast<MiniMC::int16_t,MiniMC::uint16_t> (std::get<1> (GetParam()));
  MiniMC::int16_t expected = std::get<2> (GetParam());
  auto res =MiniMC::bit_cast<MiniMC::uint16_t,MiniMC::int16_t> (MiniMC::Support::Op<MiniMC::Support::TAC::SDiv> (left,right)); 
  EXPECT_EQ(res,expected);
}




INSTANTIATE_TEST_CASE_P(
	SDivisionsFor16BitNumbers,
        SDiv16,
        ::testing::Values(
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(4,2,2)
			  
			  
			  )
			);


class SDiv32  :public ::testing::TestWithParam<std::tuple<MiniMC::int32_t,MiniMC::int32_t,MiniMC::int32_t>> {  
};

TEST_P(SDiv32, KK) {
  MiniMC::BV32 left = MiniMC::bit_cast<MiniMC::int32_t,MiniMC::uint32_t> (std::get<0> (GetParam()));
  MiniMC::BV32 right = MiniMC::bit_cast<MiniMC::int32_t,MiniMC::uint32_t> (std::get<1> (GetParam()));
  MiniMC::int32_t expected = std::get<2> (GetParam());
  auto res =MiniMC::bit_cast<MiniMC::uint32_t,MiniMC::int32_t> (MiniMC::Support::Op<MiniMC::Support::TAC::SDiv> (left,right)); 
  EXPECT_EQ(res,expected);
}


INSTANTIATE_TEST_CASE_P(
        SDivisionsFor32BitNumbers,
        SDiv32,
        ::testing::Values(
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(4,2,2)
			  
			  )
			);


class SDiv64  :public ::testing::TestWithParam<std::tuple<MiniMC::int64_t,MiniMC::int64_t,MiniMC::int64_t>> {  
};

TEST_P(SDiv64, KK) {
  MiniMC::uint64_tBV64 left = MiniMC::bit_cast<MiniMC::int64_t,MiniMC::uint64_t> (std::get<0> (GetParam()));
  MiniMC::uint64_tBV64 right = MiniMC::bit_cast<MiniMC::int64_t,MiniMC::uint64_t> (std::get<1> (GetParam()));
  MiniMC::int64_t expected = std::get<2> (GetParam());
  auto res =MiniMC::bit_cast<MiniMC::uint64_t,MiniMC::int64_t> (MiniMC::Support::Op<MiniMC::Support::TAC::SDiv> (left,right)); 
  EXPECT_EQ(res,expected);
}

INSTANTIATE_TEST_CASE_P(
        SDivisionsFor64BitNumbers,
        SDiv64,
        ::testing::Values(
			  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(4,2,2)
			  
			  )
			);

