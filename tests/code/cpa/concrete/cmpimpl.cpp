#include "cmpimpl.hpp"
#include "gtest/gtest.h"


class AddUi8  :public ::testing::TestWithParam<std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>> {  
};

TEST_P(AddUi8, KK) {
  MiniMC::uint8_t left = std::get<0> (GetParam());
  MiniMC::uint8_t right = std::get<1> (GetParam());
  MiniMC::uint8_t expected = std::get<2> (GetParam());

  MiniMC::Util::Array larr (sizeof(MiniMC::uint8_t));
  MiniMC::Util::Array rarr (sizeof(MiniMC::uint8_t));
  larr.template set (0,left);
  rarr.template set (0,right);
  auto res = MiniMC::CPA::Concrete::Stepcmpexec<MiniMC::Model::InstructionCode::ICMP_EQ> (larr,rarr);
  
  EXPECT_EQ(res.template read<MiniMC::uint8_t> (),expected);
}

INSTANTIATE_TEST_CASE_P(
						TACAddOperations,
						AddUi8,
						::testing::Values(
										  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(4,2,0),
										  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(10,2,0),
										  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(255,2,0),
										  std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>(0,2,0)
										  
										  )
						);
