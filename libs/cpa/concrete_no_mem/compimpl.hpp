#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/cmps.hpp"
#include "support/exceptions.hpp"
#include "support/storehelp.hpp"

#include "register.hpp"
#include "stack.hpp"
#include "state.hpp"


namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      template<MiniMC::Model::InstructionCode,class T>
      struct CompExec {
	static OutRegister execute (const InRegister& left, const InRegister& right){
	  throw MiniMC::Support::Exception ("Bug");
	}
      };

#define OPS			\
      X(ICMP_SGT,sgt)		\
      X(ICMP_UGT,ugt)		\
      X(ICMP_SGE,sgeq)		\
      X(ICMP_UGE,ugeq)		\
      X(ICMP_SLT,slt)		\
      X(ICMP_ULT,ult)		\
      X(ICMP_SLE,sleq)		\
      X(ICMP_ULE,uleq)		\
      X(ICMP_EQ,eq)		\
      X(ICMP_NEQ,neq)		\
      

#define X(OPC,func)				\
      template<class T>							\
      struct CompExec<MiniMC::Model::InstructionCode::OPC,T> {		\
	static OutRegister execute (const InRegister& left, const InRegister& right) { \
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[1]); \
	  MiniMC::saveHelper<uint8_t> (hh.get (),1) = MiniMC::Support::func(left.template get<T> (),right.template get<T> ()); \
	  return OutRegister(hh,1);				\
	}								\
      };								\

      OPS
#undef OPS
#undef X
      template<MiniMC::Model::InstructionCode opc>
      struct CMPRedirect{
	using i8 = CompExec<opc,MiniMC::uint8_t>;
	using i16 = CompExec<opc,MiniMC::uint16_t>;
	using i32 = CompExec<opc,MiniMC::uint32_t>;
	using i64 = CompExec<opc,MiniMC::uint64_t>;
	using boolean = CompExec<opc,MiniMC::uint8_t>;
      };
      
    }
  }
}
