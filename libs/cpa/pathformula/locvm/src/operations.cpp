#include "pathvm/operations.hpp"
#include "minimc/smt/smtconstruction.hpp"
#include "pathvm/value.hpp"
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
      template <class T>						\
      T Operations::OP(const T& l, const T& r) const {			\
	return builder.buildTerm(SMTLib::Ops::BV##OP, {l.getTerm(), r.getTerm()}); \
      }
      INTOPS
#undef X

#define X(OP, SMTOP)							\
      template <class T>						\
      Value::Bool Operations::OP(const T& l, const T& r) const {		\
	return builder.buildTerm(SMTLib::Ops::SMTOP, {l.getTerm(), r.getTerm()}); \
      }
      CMPOPS
#undef X
      Value::Bool Operations::BoolNegate(const Value::Bool& bv) const {
        return BoolValue(builder.buildTerm(SMTLib::Ops::Not, {bv.getTerm()}));
      }
      
#define X(OP)								\
	template Value::I8 Operations::OP(const Value::I8& l, const I8Value& r) const; \
  template I16Value Operations::OP(const I16Value& l, const I16Value& r) const; \
  template I32Value Operations::OP(const I32Value& l, const I32Value& r) const; \
  template I64Value Operations::OP(const I64Value& l, const I64Value& r) const;

      INTOPS
#undef X
#define X(OP, SMTOP)                                                                                        \
      template Value::Bool Operations::OP(const Value::I8& l, const Value::I8& r) const; \
      template Value::Bool Operations::OP(const I16Value& l, const I16Value& r) const; \
      template Value::Bool Operations::OP(const I32Value& l, const I32Value& r) const; \
      template Value::Bool Operations::OP(const I64Value& l, const I64Value& r) const;
      
      CMPOPS
#undef X

      template <class T>
      Value::Pointer Operations::PtrAdd(const Value::Pointer& ptrvalue, const T& addend) const {
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

      template <class T>
      Value::Pointer Operations::PtrSub(const Value::Pointer& ptrvalue, const T& addend)  const{
	SMTLib::Term_ptr extended;
	constexpr std::size_t ptrsize = PointerValue::intbitsize ();
	if constexpr (T::intbitsize () == ptrsize) {
	  extended = addend.getTerm ();
	}
	else if constexpr (T::intbitsize () <= ptrsize)
	  extended = builder.buildTerm(SMTLib::Ops::ZExt,{addend.getTerm ()},{ptrsize - T::intbitsize ()});
	else
	  extended = builder.buildTerm(SMTLib::Ops::Extract,{addend.getTerm ()},{ptrsize-1,0});
	auto res = builder.buildTerm(SMTLib::Ops::BVSub,{ptrvalue.getTerm (),extended});
	return res;
	
      }
      
      template <class T>
      T Operations::ExtractBaseValue(const Value::Aggregate& aggr, const std::size_t offset) const {
	auto aggrTerm = aggr.getTerm();
	return BVHelper{builder,aggrTerm,aggr.size()}.extractBytes<NativeLoad ()> (offset,T::intbitsize () /8);
	
      }
      
      Value::Aggregate Operations::ExtractAggregateValue(const Value::Aggregate& aggr, const std::size_t offset, std::size_t size)  const{
	auto aggrTerm = aggr.getTerm();
        
	return {BVHelper{builder,aggrTerm,aggr.size()}.extractBytes<LoadType::Straight> (offset,size),size};
	
      }

      template <class T>
      Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate& aggr, const std::size_t offset, const T& val) const {
	return {BVHelper{builder,aggr.getTerm (),aggr.size()}.storeBytes<NativeLoad ()> (offset,val.getTerm (),T::intbitsize() / 8),aggr.size ()};
      }


      Value::Aggregate Operations::InsertAggregateValue(const Value::Aggregate& aggr, const std::size_t offset, const Value::Aggregate& val) const {
	return {BVHelper{builder,aggr.getTerm (),aggr.size()}.storeBytes<LoadType::Straight> (offset,val.getTerm (),val.size()),aggr.size ()};
      }

      template Value::Pointer Operations::PtrAdd (const Value::Pointer&,const Value::I8&) const;
      template Value::Pointer Operations::PtrAdd (const Value::Pointer&,const Value::I16&) const;
      template Value::Pointer Operations::PtrAdd (const Value::Pointer&,const Value::I32&) const;
      template Value::Pointer Operations::PtrAdd (const Value::Pointer&,const Value::I64&) const;
      template Value::Pointer Operations::PtrSub (const Value::Pointer&,const Value::I8&) const;
      template Value::Pointer Operations::PtrSub (const Value::Pointer&,const Value::I16&) const;
      template Value::Pointer Operations::PtrSub (const Value::Pointer&,const Value::I32&) const;
      template Value::Pointer Operations::PtrSub (const Value::Pointer&,const Value::I64&) const;
      
      
      template Value::I8 Operations::ExtractBaseValue(const Value::Aggregate&, std::size_t) const;
      template Value::I16 Operations::ExtractBaseValue(const Value::Aggregate&, std::size_t) const;
      template Value::I32 Operations::ExtractBaseValue(const Value::Aggregate&, std::size_t) const;
      template Value::I64 Operations::ExtractBaseValue(const Value::Aggregate&, std::size_t) const;
      template Value::Pointer Operations::ExtractBaseValue(const Value::Aggregate&, std::size_t) const;
      template Value::Bool Operations::ExtractBaseValue(const Value::Aggregate&, std::size_t) const;
      
      template Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate&, std::size_t, const Value::I8&) const;
      template Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate&, std::size_t, const Value::I16&) const;
      template Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate&, std::size_t, const Value::I32&) const;
      template Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate&, std::size_t, const Value::I64&) const;

      template Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate&, std::size_t, const Value::Pointer&) const;
      template Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate&, std::size_t, const Value::Pointer32&) const;  
      template Value::Aggregate Operations::InsertBaseValue(const Value::Aggregate&, std::size_t, const Value::Bool&) const;

    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
