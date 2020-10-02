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
      struct PredExec {
	static void execute (const InRegister& left, const InRegister& right){
	  throw MiniMC::Support::Exception ("Bug");
	}
      };

#define OPS			\
      X(PRED_ICMP_SGT,sgt)		\
      X(PRED_ICMP_UGT,ugt)		\
      X(PRED_ICMP_SGE,sgeq)		\
      X(PRED_ICMP_UGE,ugeq)		\
      X(PRED_ICMP_SLT,slt)		\
      X(PRED_ICMP_ULT,ult)		\
      X(PRED_ICMP_SLE,sleq)		\
      X(PRED_ICMP_ULE,uleq)		\
      X(PRED_ICMP_EQ,eq)		\
      X(PRED_ICMP_NEQ,neq)		\
      

#define X(OPC,func)				\
      template<class T>							\
      struct PredExec<MiniMC::Model::InstructionCode::OPC,T> {		\
	static void execute (const InRegister& left, const InRegister& right) { \
	  if (!MiniMC::Support::func(left.template get<T> (),right.template get<T> ())) \
	    throw MiniMC::Support::AssumeViolated ();			\
	}								\
      };								\

      OPS
#undef OPS
#undef X
      template<MiniMC::Model::InstructionCode opc>
      struct PredRedirect{
	using i8 = PredExec<opc,MiniMC::uint8_t>;
	using i16 = PredExec<opc,MiniMC::uint16_t>;
	using i32 = PredExec<opc,MiniMC::uint32_t>;
	using i64 = PredExec<opc,MiniMC::uint64_t>;
	using boolean = PredExec<opc,MiniMC::uint8_t>;
      };
      
    }
  }
}
