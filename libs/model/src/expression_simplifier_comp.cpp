#include "minimc/model/variables.hpp"
#include "minimc/host/operataions.hpp"
#include "minimc/host/casts.hpp"

namespace MiniMC {
  namespace Model {


#define OPS								\
      X(SGt,SGT)							\
      X(UGt,UGT)							\
      X(SGe,SGE)							\
      X(UGe,UGE)								\
      X(SLt,SLT)							\
      X(ULt,ULT)							\
      X(SLe,SLE)							\
      X(ULe,ULE)							\
      X(Eq,EQ)								\
      X(NEq,NEQ)								\

#define X(OP,COM)								\
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::OP##Expr& i) const { \
      auto op1 = Simplify (i.op1());					\
      auto op2 = Simplify (i.op2());					\
      									\
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {	\
	  []<typename T>(T& l, T& r)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> { \
	    return MiniMC::Model::Bool::make (MiniMC::Host::Op<MiniMC::Host::CMP::COM> (l.getValue(),r.getValue())); \
	  },								\
	    [&op1,&op2](auto&, auto&)->MiniMC::Model::Value_ptr{return MiniMC::Model::makeExpr<MiniMC::Model::OP##Expr> (op1,op2);} \
	    },								\
	*op1,*op2);							\
      									\
    }
    
    OPS
#undef X
#undef OPS    


  }
}
