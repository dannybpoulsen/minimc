#include "vm/pathformula/operations.hpp"
#include "smt/smtconstruction.hpp"
#include "vm/pathformula/value.hpp"
#include "smt/builder.hpp"
#include "aggrhelper.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {

      
#define INTOPS \
  X(Add)       \
  X(Sub)       \
  X(Mul)       \
  X(UDiv)      \
  X(SDiv)      \
  X(LShl)      \
  X(LShr)      \
  X(AShr)      \
  X(And)       \
  X(Or)        \
  X(Xor)

#define CMPOPS   \
  X(SGt, BVSGt)  \
  X(SLt, BVSLt)  \
  X(SLe, BVSLEq) \
  X(SGe, BVSGEq) \
  X(UGt, BVUGt)  \
  X(ULt, BVULt)  \
  X(ULe, BVULEq) \
  X(UGe, BVUGEq) \
  X(Eq, Equal)   \
  X(NEq, NotEqual)

#define X(OP)								\
      template<class Value>						\
      template <class T>						\
      T Operations<Value>::OP(const T& l, const T& r) {			\
	return builder.buildTerm(SMTLib::Ops::BV##OP, {l.getTerm(), r.getTerm()}); \
      }
      INTOPS
#undef X

#define X(OP, SMTOP)							\
      template<class Value>						\
      template <class T>						\
      Value::Bool Operations<Value>::OP(const T& l, const T& r) {		\
	return builder.buildTerm(SMTLib::Ops::SMTOP, {l.getTerm(), r.getTerm()}); \
      }
      CMPOPS
#undef X
      template<class Value>
      Value::Bool Operations<Value>::BoolNegate(const Value::Bool& bv) {
        return BoolValue(builder.buildTerm(SMTLib::Ops::Not, {bv.getTerm()}));
      }
      
#define X(OP)								\
	template PathFormulaVMVal::I8 Operations<PathFormulaVMVal>::OP(const PathFormulaVMVal::I8& l, const I8Value& r); \
  template I16Value Operations<PathFormulaVMVal>::OP(const I16Value& l, const I16Value& r); \
  template I32Value Operations<PathFormulaVMVal>::OP(const I32Value& l, const I32Value& r); \
  template I64Value Operations<PathFormulaVMVal>::OP(const I64Value& l, const I64Value& r);

      INTOPS
#undef X
#define X(OP, SMTOP)                                                                                        \
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::OP(const PathFormulaVMVal::I8& l, const PathFormulaVMVal::I8& r); \
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::OP(const I16Value& l, const I16Value& r); \
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::OP(const I32Value& l, const I32Value& r); \
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::OP(const I64Value& l, const I64Value& r);
      
      CMPOPS
#undef X

      template<class Value>
      template <class T>
      Value::Pointer Operations<Value>::PtrAdd(const Value::Pointer& ptrvalue, const T& addend) {
	SMTLib::Term_ptr extended;
	constexpr std::size_t ptrsize = PointerValue::intbitsize ();
	if constexpr (T::intbitsize () == ptrsize) {
	  extended = addend.getTerm ();
	}
	else if constexpr (T::intbitsize () <= ptrsize)
	  extended = builder.buildTerm(SMTLib::Ops::ZExt,{addend.getTerm ()},{ptrsize - T::intbitsize ()});
	else
	  extended = builder.buildTerm(SMTLib::Ops::Extract,{addend.getTerm ()},{ptrsize-1,0});
	auto res = builder.buildTerm(SMTLib::Ops::BVAdd,{ptrvalue.getTerm (),extended});
	return res;
	
      }
	
      template<class Value>
      template <class T>
      T Operations<Value>::ExtractBaseValue(const Value::Aggregate& aggr, const std::size_t offset) {
	auto aggrTerm = aggr.getTerm();
	return BVHelper{builder,aggrTerm,aggr.size()}.extractBytes<NativeLoad ()> (offset,T::intbitsize () /8);
	
      }
      
      template<class Value>
      Value::Aggregate Operations<Value>::ExtractAggregateValue(const Value::Aggregate& aggr, const std::size_t offset, std::size_t size) {
	auto aggrTerm = aggr.getTerm();
        
	return {BVHelper{builder,aggrTerm,aggr.size()}.extractBytes<LoadType::Straight> (offset,size),size};
	
      }

      template<class Value>
      template <class T>
      Value::Aggregate Operations<Value>::InsertBaseValue(const Value::Aggregate& aggr, const std::size_t offset, const T& val) {
	return {BVHelper{builder,aggr.getTerm (),aggr.size()}.storeBytes<NativeLoad ()> (offset,val.getTerm (),T::intbitsize() / 8),aggr.size ()};
      }

      template<class Value>
      Value::Aggregate Operations<Value>::InsertAggregateValue(const Value::Aggregate& aggr, const std::size_t offset, const Value::Aggregate& val) {
	return {BVHelper{builder,aggr.getTerm (),aggr.size()}.storeBytes<LoadType::Straight> (offset,val.getTerm (),val.size()),aggr.size ()};
      }

      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::PtrAdd (const PathFormulaVMVal::Pointer&,const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::PtrAdd (const PathFormulaVMVal::Pointer&,const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::PtrAdd (const PathFormulaVMVal::Pointer&,const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::PtrAdd (const PathFormulaVMVal::Pointer&,const PathFormulaVMVal::I64&);
      
      
      template PathFormulaVMVal::I8 Operations<PathFormulaVMVal>::ExtractBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t);
      template PathFormulaVMVal::I16 Operations<PathFormulaVMVal>::ExtractBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t);
      template PathFormulaVMVal::I32 Operations<PathFormulaVMVal>::ExtractBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t);
      template PathFormulaVMVal::I64 Operations<PathFormulaVMVal>::ExtractBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::ExtractBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t);
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::ExtractBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t);
      
      template PathFormulaVMVal::Aggregate Operations<PathFormulaVMVal>::InsertBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t, const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Aggregate Operations<PathFormulaVMVal>::InsertBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t, const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Aggregate Operations<PathFormulaVMVal>::InsertBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t, const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Aggregate Operations<PathFormulaVMVal>::InsertBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t, const PathFormulaVMVal::I64&);

      template PathFormulaVMVal::Aggregate Operations<PathFormulaVMVal>::InsertBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t, const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::Aggregate Operations<PathFormulaVMVal>::InsertBaseValue(const PathFormulaVMVal::Aggregate&, std::size_t, const PathFormulaVMVal::Bool&);

      template class Operations<PathFormulaVMVal>;
      
    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
