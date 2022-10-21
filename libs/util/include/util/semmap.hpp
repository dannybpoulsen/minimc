#ifndef _SEMMAP__
#define _SEMMAP__

#include <type_traits>

#include "support/operataions.hpp"
#include "model/instructions.hpp"



namespace MiniMC {
  namespace Util {
    template<MiniMC::Model::InstructionCode opc>
    constexpr std::enable_if_t<MiniMC::Model::InstructionData<opc>::isTAC,MiniMC::Support::TAC> InstructionCodeToOperation ();

    template<MiniMC::Model::InstructionCode opc>
    constexpr  std::enable_if_t<MiniMC::Model::InstructionData<opc>::isComparison,MiniMC::Support::CMP> InstructionCodeToOperation ();

    
#define TACMAPDEF					\
  X(Add,Add)						\
  X(Sub,Sub)						\
  X(Mul,Mul)						\
  X(UDiv,UDiv)						\
  X(SDiv,SDiv)						\
  X(Shl,Shl)						\
  X(LShr,LShr)						\
  X(AShr,AShr)						\
  X(And,And)						\
  X(Or,Or)						\
  X(Xor,Xor)

#define CMPMAPDEF							\
  X(ICMP_SGT,SGT)							\
  X(ICMP_UGT,UGE)							\
  X(ICMP_SGE,SGE)							\
  X(ICMP_UGE,UGE)							\
  X(ICMP_SLT,SLT)							\
  X(ICMP_ULT,ULT)							\
  X(ICMP_SLE,SLE)							\
  X(ICMP_ULE,ULE)							\
  X(ICMP_EQ,EQ)								\
  X(ICMP_NEQ,NEQ)


#define X(FROM, TO) \
  template <>       \
  constexpr MiniMC::Support::TAC InstructionCodeToOperation<MiniMC::Model::InstructionCode::FROM>() { \
    return MiniMC::Support::TAC::TO;					\
  }

TACMAPDEF
#undef X


#define X(FROM, TO) \
  template <>       \
  constexpr MiniMC::Support::CMP InstructionCodeToOperation<MiniMC::Model::InstructionCode::FROM>() { \
    return MiniMC::Support::CMP::TO;					\
  }

CMPMAPDEF
#undef X

    
  }
}

#endif
