#include "minimc/smt/smtconstruction.hpp"
#include "minimc/smt/smt.hpp"
#include "smt/context.hpp"
#include "minimc/support/exceptions.hpp"
#include <vector>

namespace MiniMC {
  namespace Support {
    namespace SMT {

      SMTLib::Context_ptr SMTDescr::makeContext() const {
	if (r)
	  return r->getFunction()();
	else
	  throw MiniMC::Support::ConfigurationException ("No SMT Solver selected");
      }

      std::string SMTDescr::name () const { return ( r != nullptr) ? r->getName() : "None"; }
      std::string SMTDescr::descr () const { return (r != nullptr) ? r->getDescritpion() : "No SMT Solver selected"; }

            
      template <class Iterator>
      void getBackends(Iterator it) {
        for (auto& itt : SMTLib::getSMTBackends()) {
          it = itt;
        }
      }

      SMTSolverRepository::SMTSolverRepository () {
	getBackends (std::back_inserter(descr));
      }

      SMTSolverRepository& SMTSolverRepository::get () {
	static SMTSolverRepository rep;
	return rep;
      }



      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I8Integer& val) const {
	return context->getBuilder().makeBVIntConst (val.getValue(),8);
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I16Integer& val) const {
	return context->getBuilder().makeBVIntConst (val.getValue(),16);
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I32Integer& val) const {
	return context->getBuilder().makeBVIntConst (val.getValue(),32);
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I64Integer& val) const {
	auto term = context->getBuilder().makeBVIntConst (val.getValue(),64); 
	return term;
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Bool& val) const {
	return context->getBuilder().makeBoolConst (val.getValue());
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Pointer& val) const {
	auto pointer = val.getValue ();
	auto& builder = context->getBuilder();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		<< builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		<< builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	auto term = chainer.getTerm ();
	return term;;
	
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Pointer32& val) const {
	auto pointer = val.getValue ();
	auto& builder = context->getBuilder();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		<< builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		<< builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	return chainer.getTerm ();
	
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::AggregateConstant& val) const {
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&context->getBuilder()};
	for (auto byte : val.getData().get_direct_access()) {
	  chainer >> (context->getBuilder().makeBVIntConst(byte, 8));
	}
	return chainer.getTerm();
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Register&) const {
	throw MiniMC::Support::Exception ("Registers should not be present in SMT-Translated expressions");
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Undef& und) const {
	auto type = und.getType ();
	std::stringstream str;
        str << "Var" << ++next;
	auto& builder = context->getBuilder();
	
	switch (type->getTypeID())  {
	case MiniMC::Model::TypeID::Bool:
	  return builder.makeVar(builder.makeSort(SMTLib::SortKind::Bool, {}), str.str());
	case MiniMC::Model::TypeID::Pointer32:
	  return builder.makeVar(builder.makeBVSort(32), str.str());
          
	  
	case MiniMC::Model::TypeID::Pointer:
	  return builder.makeVar(builder.makeBVSort(64), str.str());
	case MiniMC::Model::TypeID::I8:
	  return builder.makeVar(builder.makeBVSort(8), str.str());
	case MiniMC::Model::TypeID::I16:
	  return builder.makeVar(builder.makeBVSort(16), str.str());
	case MiniMC::Model::TypeID::I32:
	  return builder.makeVar(builder.makeBVSort(32), str.str());
	case MiniMC::Model::TypeID::I64:
	  return builder.makeVar(builder.makeBVSort(64), str.str());
	case MiniMC::Model::TypeID::Aggregate:
	  return builder.makeVar(builder.makeBVSort(8 * type->getSize()), str.str());
	case MiniMC::Model::TypeID::Memory: {
	  auto arr_sort = builder.makeSort(
					   SMTLib::SortKind::Array, {builder.makeBVSort(64),
								     builder.makeBVSort(8)});
	  return builder.makeVar(arr_sort, str.str());
	}
	default:
	  throw MiniMC::Support::Exception ("Weird situation");
	}
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::SymbolicConstant&) const {
	throw MiniMC::Support::Exception ("SymbolicConstants should not be present in SMT-Translated expressions");
      }

#define OPS					\
      X(Add)					\
      X(Sub)					\
      X(Mul)					\
      X(UDiv)					\
      X(SDiv)					\
      X(LShl)					\
      X(LShr)					\
      X(AShr)					\
      X(And)					\
      X(Or)					\
      X(Xor)

#define X(OP)								\
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::OP##Expr& expr) const { \
	auto left = Translate (expr.op1());				\
	auto right = Translate (expr.op2());				\
	return context->getBuilder().buildTerm (SMTLib::Ops::BV##OP,{left,right}); \
      }
      
      OPS
#undef OPS
#undef X
      
#define OPS   \
      X(SGt, BVSGt)  \
      X(SLt, BVSLt)  \
      X(SLe, BVSLEq) \
      X(SGe, BVSGEq) \
      X(UGt, BVUGt)  \
      X(ULt, BVULt)  \
      X(ULe, BVULEq) \
      X(UGe, BVUGEq) \
      X(Eq, Equal)   \
      X(NEq, NotEqual)\
      X(LogAnd, And)    
      
#define X(OP,SMTOP)							\
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::OP##Expr& expr) const { \
	auto left = Translate (expr.op1());				\
	auto right = Translate (expr.op2());				\
	return context->getBuilder().buildTerm (SMTLib::Ops::SMTOP,{left,right}); \
      }

OPS
      
#undef OPS
#undef X
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::NotExpr& expr) const {
	auto translat = Translate(expr.op1());
	return context->getBuilder().buildTerm (SMTLib::Ops::BVNot, {translat});
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::LogNotExpr& expr) const {
	auto translat = Translate(expr.op1());
	return context->getBuilder().buildTerm (SMTLib::Ops::Not, {translat});
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::TruncExpr& expr) const {
	auto fromType = expr.getFrom ().getType();
	auto toType = expr.getToType();
	std::size_t highbit = toType->getSize()*8 -1;
	return context->getBuilder().buildTerm (SMTLib::Ops::Extract, {Translate(expr.getFrom())},{highbit,0});
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::BitCastExpr& expr) const {
	return Translate(expr.getFrom());
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::ZExtExpr& expr) const {
	auto fromType = expr.getFrom ().getType();
	auto toType = expr.getToType();  
	if (fromType->getTypeID() != MiniMC::Model::TypeID::Bool) {
	  assert(fromType);
	  assert(toType);
	  std::size_t bits = toType->getSize()*8 - fromType->getSize()*8;
	  return context->getBuilder().buildTerm(SMTLib::Ops::ZExt,{Translate(expr.getFrom())},{bits});
	}
	else {
	  auto zeros = context->getBuilder().makeBVIntConst(0, toType->getSize()*8);
	  auto ones = context->getBuilder().makeBVIntConst(1, toType->getSize()*8);
	  auto res = context->getBuilder().buildTerm(SMTLib::Ops::ITE, {Translate(expr.getFrom()), ones, zeros});
	  return res;
	  
	}
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::SExtExpr& expr) const {
	auto fromType = expr.getFrom ().getType();
	auto toType = expr.getToType();
	if (fromType->getTypeID() != MiniMC::Model::TypeID::Bool) {
	  std::size_t bits = toType->getSize()*8 - fromType->getSize()*8;
	  return  context->getBuilder().buildTerm(SMTLib::Ops::SExt,{Translate(expr.getFrom())},{bits});
	}
	else {
	  auto zeros = context->getBuilder().makeBVIntConst(0, toType->getSize()*8);
	  auto ones = context->getBuilder().makeBVIntConst(~0, toType->getSize()*8);
	  return context->getBuilder().buildTerm(SMTLib::Ops::ITE, {Translate(expr.getFrom()), ones, zeros});
	  
	}
	    
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrToIntExpr&) const {
	throw MiniMC::Support::Exception ("PtrToIntExpr should not be present in SMT-Translated expressions");
     
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::IntToPtrExpr&) const {
	throw MiniMC::Support::Exception ("IntToPtrExpr should not be present in SMT-Translated expressions");
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::IntToBoolExpr& expr) const {
	auto& builder = context->getBuilder();
	auto tt = builder.makeBoolConst(true);
	auto ff = builder.makeBoolConst(false);
        auto zeros = builder.makeBVIntConst(0, expr.getFrom().getType()->getSize()*8);
        auto eq = builder.buildTerm(SMTLib::Ops::Equal, {Translate(expr.getFrom()), zeros});
        return builder.buildTerm(SMTLib::Ops::ITE, {eq, ff,tt});
	
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrAddExpr& expr) const {
	return context->getBuilder().buildTerm(SMTLib::Ops::BVAdd,{Translate(expr.ptr()),Translate(expr.skipsize())});
      }

      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrSubExpr& expr) const {
	return context->getBuilder().buildTerm(SMTLib::Ops::BVSub,{Translate(expr.ptr()),Translate(expr.skipsize())});
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::ExtractValueExpr&) const {
	throw MiniMC::Support::Exception ("ExtractValueValueExpr not implemented");
      }

      
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::InsertValueExpr& insertexpr) const {
	auto aggr = Translate (insertexpr.aggregate());
	MiniMC::BV64 offset = MiniMC::Model::visitValue(
						MiniMC::Support::Overload {
						  [](const MiniMC::Model::I64Integer& v)->MiniMC::BV64 {return v.getValue();},
						    MiniMC::Support::Error<MiniMC::BV64> {}
						},
						insertexpr.offset()
							      );
	auto insertee = Translate(insertexpr.insertee());

	if (insertexpr.insertee().getType()->getTypeID () != MiniMC::Model::TypeID::I8) {
	  throw MiniMC::Support::Exception ("Can only insert BV8 into aggregates");
	}
	auto termsize = insertexpr.aggregate().getType()->getSize();
	MiniMC::Support::SMT::BVHelper helper{context->getBuilder(),aggr,termsize};
	SMTLib::Term_ptr pre = nullptr;
	SMTLib::Term_ptr post = nullptr;

	if (offset == 0) {
	  post = helper.extractBytes (1,termsize -1);  
	}

	else if (offset + 1 == termsize) {
	  pre = helper.extractBytes(0,termsize-1);
	}
	
	else {
	  pre = helper.extractBytes (0,offset);
	  post = helper.extractBytes (offset+1,termsize-offset-1);
	  
	}
	
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer {&context->getBuilder()};
	if (pre)
	  chainer >>  pre;
	chainer >> insertee;
	if (post)
	chainer >> post;
	/*for (std::size_t i = 0 ; i  < insertexpr.aggregate().getType()->getSize(); ++i) {
	  
	  if (i != offset)
	    chainer >> helper.extractByte(i);
	  else {
	    chainer >> insertee;
	  }
	  }*/
	auto term = chainer.getTerm(); 
	return term;;
	//throw MiniMC::Support::Exception ("InsertValueValueExpr not implemented");
      
      }

      SMTLib::Term_ptr Translator::operator()(const MiniMC::Model::StoreExpr& e) const {
        SMTLib::Term_ptr addr = Translate(e.addr());
	SMTLib::Term_ptr mem = Translate(e.storeto());	
	SMTLib::Term_ptr storee = Translate(e.storee());
 
	if (e.storee().getType()->getTypeID() == MiniMC::Model::TypeID::I8) {
	  return context->getBuilder().buildTerm (SMTLib::Ops::Store,{mem,addr,storee});
	}
	else 
	  throw MiniMC::Support::Exception ("StoreExpr not implemented");
	
	
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::LoadExpr& e) const {
	auto type = e.getToType();
	auto addr = Translate(e.addr());
	auto mem = Translate(e.mem());
	if (type->getTypeID() == MiniMC::Model::TypeID::I8) {
	  return context->getBuilder().buildTerm (SMTLib::Ops::Select,{mem,addr});
	}
	else 
	  throw MiniMC::Support::Exception ("LoadExpr not implemented for non I8 Types");
	
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Ptr32ToPtrExpr&) const {
	throw MiniMC::Support::Exception ("Ptr32ToPtrExpr not implemented");
      
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrToPtr32Expr&) const {
	throw MiniMC::Support::Exception ("PtrToPtr32Expr not implemented");	
      }

      
      
    } // namespace SMT
  }   // namespace Support
} // namespace MiniMC
