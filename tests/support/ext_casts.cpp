#include <tuple>

#include "support/casts.hpp"
#include "support/types.hpp"
#include "gtest/gtest.h"

template<typename From,typename To>
class ZExt  :public ::testing::TestWithParam<std::tuple<From,To>> {  
public:
  using Inp = std::tuple<From,To>;
  void test () {
    From left = std::get<0> (this->GetParam());
    To expected = std::get<1> (this->GetParam());
    To res = MiniMC::Support::zext<From,To> (left);
    EXPECT_EQ(res, expected);
  }
};


#define MINIMC_TESTS					\
  X(ZExt8_16,MiniMC::uint8_t,MiniMC::uint16_t)		\
  

#define X(NAME,FROM,TO)				\
  using NAME = ZExt<FROM,TO>;						\
  TEST_P(NAME , KK) {							\
    this->test ();							\
  }

MINIMC_TESTS
#undef MINIMC_TESTS
#undef X

INSTANTIATE_TEST_CASE_P(ZExt8_16,
			ZExt8_16,
			::testing::Values(
					  ZExt8_16::Inp (1,1)
					  
					  )
			);

template<typename From,typename To>
class SExt  :public ::testing::TestWithParam<std::tuple<From,To>> {  
public:
  using Inp = std::tuple<From,To>;
  void test () {
    From left = std::get<0> (this->GetParam());
    To expected = std::get<1> (this->GetParam());
    To res = MiniMC::Support::sext<From,To> (left);
    EXPECT_EQ(res, expected);
  }
};


#define MINIMC_TESTS					\
  X(SExt8_16,MiniMC::int8_t,MiniMC::int16_t)		\
  

#define X(NAME,FROM,TO)				\
  using NAME = SExt<FROM,TO>;						\
  TEST_P(NAME , KK) {							\
    this->test ();							\
  }

MINIMC_TESTS
#undef X
#undef MINIMC_TESTS

INSTANTIATE_TEST_CASE_P(SExt8_16,
			SExt8_16,
			::testing::Values(
					  SExt8_16::Inp (-1,-1)
					  
					  )
			);


template<typename From,typename To>
class Trunc  :public ::testing::TestWithParam<std::tuple<From,To>> {  
public:
  using Inp = std::tuple<From,To>;
  void test () {
    From left = std::get<0> (this->GetParam());
    To expected = std::get<1> (this->GetParam());
    To res = MiniMC::Support::trunc<From,To> (left);
    EXPECT_EQ(res, expected);
  }
};


#define MINIMC_TESTS					\
  X(Trunc16_8,MiniMC::int16_t,MiniMC::int8_t)		\
  

#define X(NAME,FROM,TO)				\
  using NAME = Trunc<FROM,TO>;						\
  TEST_P(NAME , KK) {							\
    this->test ();							\
  }

MINIMC_TESTS
#undef X
#undef MINIMC_TESTS

INSTANTIATE_TEST_CASE_P(Trunc16_8,
			Trunc16_8,
			::testing::Values(
					  Trunc16_8::Inp (512,256)
					  
					  )
			);

