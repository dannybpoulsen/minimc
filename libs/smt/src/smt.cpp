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



      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I8Integer& val){
	return context->getBuilder().makeBVIntConst (val.getValue(),8);
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I16Integer& val){
	return context->getBuilder().makeBVIntConst (val.getValue(),16);
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I32Integer& val){
	return context->getBuilder().makeBVIntConst (val.getValue(),32);
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::I64Integer& val){
	return context->getBuilder().makeBVIntConst (val.getValue(),64);
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Bool& val) {
	return context->getBuilder().makeBoolConst (val.getValue());
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Pointer& val){
	auto pointer = val.getValue ();
	auto& builder = context->getBuilder();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		<< builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		<< builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	return chainer.getTerm ();
	
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Pointer32& val){
	auto pointer = val.getValue ();
	auto& builder = context->getBuilder();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		<< builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		<< builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	return chainer.getTerm ();
	
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::AggregateConstant& val){
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&context->getBuilder()};
	for (auto byte : val.getData().get_direct_access()) {
	  chainer >> (context->getBuilder().makeBVIntConst(byte, 8));
	}
	return chainer.getTerm();
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Register&){
	throw MiniMC::Support::Exception ("Registers should not be present in SMT-Translated expressions");
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Undef& und){
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
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::SymbolicConstant&){
	throw MiniMC::Support::Exception ("SymbolicConstants should not be present in SMT-Translated expressions");
      }

#define OPS \
      X(Add)   \
      X(Sub)   \
      X(Mul)   \
      X(UDiv)  \
      X(SDiv)  \
      X(LShl)  \
      X(LShr)  \
      X(AShr)  \
      X(And)   \
      X(Or)    \
      X(Xor)

#define X(OP)								\
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::OP##Expr& expr){ \
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
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::OP##Expr& expr){ \
	auto left = Translate (expr.op1());				\
	auto right = Translate (expr.op2());				\
	return context->getBuilder().buildTerm (SMTLib::Ops::SMTOP,{left,right}); \
      }

OPS
      
#undef OPS
#undef X
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::NotExpr& expr){
	auto translat = Translate(expr.op1());
	return context->getBuilder().buildTerm (SMTLib::Ops::BVNot, {translat});
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::LogNotExpr& expr){
	auto translat = Translate(expr.op1());
	return context->getBuilder().buildTerm (SMTLib::Ops::Not, {translat});
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::TruncExpr& expr){
	auto fromType = expr.getFrom ().getType();
	auto toType = expr.getToType();
	std::size_t highbit = toType->getSize()*8 -1;
	return context->getBuilder().buildTerm (SMTLib::Ops::Extract, {Translate(expr.getFrom())},{highbit,0});
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::BitCastExpr& expr){
	return Translate(expr.getFrom());
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::ZExtExpr& expr){
	auto fromType = expr.getFrom ().getType();
	auto toType = expr.getToType();
	std::size_t bits = toType->getSize()*8 - fromType->getSize()*8;
	return context->getBuilder().buildTerm(SMTLib::Ops::ZExt,{Translate(expr.getFrom())},{bits});
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::SExtExpr& expr){
	auto fromType = expr.getFrom ().getType();
	auto toType = expr.getToType();
	std::size_t bits = toType->getSize()*8 - fromType->getSize()*8;
	return  context->getBuilder().buildTerm(SMTLib::Ops::SExt,{Translate(expr.getFrom())},{bits});
      }
      
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrToIntExpr&){
	throw MiniMC::Support::Exception ("PtrToIntExpr should not be present in SMT-Translated expressions");
     
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::IntToPtrExpr&){
	throw MiniMC::Support::Exception ("IntToPtrExpr should not be present in SMT-Translated expressions");
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::IntToBoolExpr& expr){
	auto& builder = context->getBuilder();
	auto tt = builder.makeBoolConst(true);
	auto ff = builder.makeBoolConst(false);
        auto zeros = builder.makeBVIntConst(0, expr.getFrom().getType()->getSize()*8);
        auto eq = builder.buildTerm(SMTLib::Ops::Equal, {Translate(expr.getFrom()), zeros});
        return builder.buildTerm(SMTLib::Ops::ITE, {eq, ff,tt});
	
      }
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrAddExpr&){}
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrSubExpr&){}
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::ExtractValueExpr&){}
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::InsertValueExpr&){}
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::StoreExpr&){}
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::LoadExpr&){}
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::Ptr32ToPtrExpr&){}
      SMTLib::Term_ptr Translator::operator() (const MiniMC::Model::PtrToPtr32Expr&){}

      
      
    } // namespace SMT
  }   // namespace Support
} // namespace MiniMC
