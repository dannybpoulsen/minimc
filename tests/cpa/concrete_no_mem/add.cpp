#include <tuple>

#include "gtest/gtest.h"
#include "model/instructions.hpp"
#include "support/types.hpp"
#include "register.hpp"
#include "instructionimpl.hpp"

template <MiniMC::Model::InstructionCode code,class T>
class FooTest : public ::testing::TestWithParam<std::tuple<MiniMC::uint8_t,MiniMC::uint8_t,MiniMC::uint8_t>> {
public: 
  void execute ()  {
    T left = std::get<0> (GetParam());
    T right = std::get<1> (GetParam());
    T expected = std::get<2> (GetParam());

    MiniMC::CPA::ConcreteNoMem::InRegister lreg (&left,sizeof(T));
    MiniMC::CPA::ConcreteNoMem::InRegister rreg (&right,sizeof(T));
    MiniMC::CPA::ConcreteNoMem::OutRegister res = MiniMC::CPA::ConcreteNoMem::TACExec<code,T>::execute (lreg,rreg);
    T K = res.template get<T> ();
    EXPECT_EQ(K,expected );
  }
};

#define MINIMC_TESTS				\
  X(AddUI8,Add,MiniMC::uint8_t)			\
  X(AddUI16,Add,MiniMC::uint16_t)			\
  X(AddUI32,Add,MiniMC::uint32_t)			\
  X(AddUI64,Add,MiniMC::uint64_t)			\
  X(SubUI8,Sub,MiniMC::uint8_t)				\
  X(SubUI16,Sub,MiniMC::uint16_t)			\
  X(SubUI32,Sub,MiniMC::uint32_t)			\
  X(SubUI64,Sub,MiniMC::uint64_t)			\
  X(UDivUI8,UDiv,MiniMC::uint8_t)			\
  X(UDivUI16,UDiv,MiniMC::uint16_t)			\
  X(UDivUI32,UDiv,MiniMC::uint32_t)			\
  X(UDivUI64,UDiv,MiniMC::uint64_t)			\
  X(SDivUI8,SDiv,MiniMC::uint8_t)			\
  X(SDivUI16,SDiv,MiniMC::uint16_t)			\
  X(SDivUI32,SDiv,MiniMC::uint32_t)			\
  X(SDivUI64,SDiv,MiniMC::uint64_t)			\
  X(ShlUI8,Shl,MiniMC::uint8_t)				\
  X(ShlUI16,Shl,MiniMC::uint16_t)			\
  X(ShlUI32,Shl,MiniMC::uint32_t)			\
  X(ShlUI64,Shl,MiniMC::uint64_t)			\
  X(LShrUI8,LShr,MiniMC::uint8_t)			\
  X(LShrUI16,LShr,MiniMC::uint16_t)			\
  X(LShrUI32,LShr,MiniMC::uint32_t)			\
  X(AShrUI64,LShr,MiniMC::uint64_t)			\
  X(AShrUI8,AShr,MiniMC::uint8_t)			\
  X(AShrUI16,AShr,MiniMC::uint16_t)			\
  X(AShrUI32,AShr,MiniMC::uint32_t)			\
  X(AndUI64,AShr,MiniMC::uint64_t)			\
  X(AndUI8,And,MiniMC::uint8_t)				\
  X(AndUI16,And,MiniMC::uint16_t)			\
  X(AndUI32,And,MiniMC::uint32_t)			\
  X(AbdUI64,And,MiniMC::uint64_t)			\
  X(OrUI8,Or,MiniMC::uint8_t)				\
  X(OrUI16,Or,MiniMC::uint16_t)				\
  X(OrUI32,Or,MiniMC::uint32_t)				\
  X(OrUI64,Or,MiniMC::uint64_t)				\
  X(XOrUI8,Xor,MiniMC::uint8_t)				\
  X(XOrUI16,Xor,MiniMC::uint16_t)			\
  X(XOrUI32,Xor,MiniMC::uint32_t)			\
  X(XOrUI64,Xor,MiniMC::uint64_t)			\
  

#define X(NAME,OP,param)						\
  using NAME = FooTest<MiniMC::Model::InstructionCode::OP,param>;	\
  TEST_P(NAME , KK) {							\
    this->execute ();							\
  }									\


MINIMC_TESTS
#undef X

using inp8 = std::tuple<MiniMC::uint8_t, MiniMC::uint8_t,MiniMC::uint8_t>;
using inp16 = std::tuple<MiniMC::uint16_t, MiniMC::uint16_t,MiniMC::uint16_t>;
using inp32 = std::tuple<MiniMC::uint32_t, MiniMC::uint32_t,MiniMC::uint32_t>;
using inp64 = std::tuple<MiniMC::uint64_t, MiniMC::uint64_t,MiniMC::uint64_t>;


INSTANTIATE_TEST_CASE_P(
	CPAConcreteValuesAdd,
        AddUI8,
        ::testing::Values(
			  inp8(4,2,6)
			  )
			);


INSTANTIATE_TEST_CASE_P(
	CPAConcreteValuesSub,
        SubUI8,
        ::testing::Values(
			  inp8(4,2,2)
			  )
			);


//typedef ::testing::Types<Input<MiniMC::uint8_t,0,0,1,MiniMC::Model::InstructionCode::Add> > MyTypes;
//INSTANTIATE_TYPED_TEST_SUITE_P(My, FooTest, MyTypes);

