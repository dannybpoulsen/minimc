#include "minimc/model/variables.hpp"
#include "minimc/host/operataions.hpp"

namespace MiniMC {
  namespace Model {

#define OPS					\
    X(Add)					\
    X(Sub)					\
    X(UDiv)					\
    X(SDiv)					\
    X(Mul)					\
    X(LShl)					\
    X(LShr)					\
    X(AShr)					\
    X(And)					\
    X(Or)					\
    X(Xor)					\

    
#define X(OP)								\
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::OP##Expr& i) const { \
      auto op1 = Simplify (i.op1());					\
    auto op2 = Simplify (i.op2());					\
									\
    return MiniMC::Model::visitValue ( MiniMC::Support::Overload {	\
	[]<typename T>(T& l, T& r)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> { \
      return T::make (MiniMC::Host::Op<MiniMC::Host::TAC::OP> (l.getValue(),r.getValue())); \
    },									\
											      [&op1,&op2](auto&, auto&)->MiniMC::Model::Value_ptr{return MiniMC::Model::makeExpr<MiniMC::Model::OP##Expr> (op1,op2);} \
											      }, \
      *op1,*op2);				\
		 \
    }		 \

    OPS
#undef OPS    
#undef X

  }
}
