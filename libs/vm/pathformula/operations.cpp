#include "vm/pathformula/operations.hpp"
#include "util/smtconstruction.hpp"
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

#define X(OP)                                                                  \
  template <class T>                                                           \
  T Operations::OP(const T& l, const T& r) {                                   \
    return builder.buildTerm(SMTLib::Ops::BV##OP, {l.getTerm(), r.getTerm()}); \
  }
      INTOPS
#undef X

#define X(OP, SMTOP)                                                          \
  template <class T>                                                          \
  Value<ValType::Bool> Operations::OP(const T& l, const T& r) {               \
    return builder.buildTerm(SMTLib::Ops::SMTOP, {l.getTerm(), r.getTerm()}); \
  }
      CMPOPS
#undef X

      BoolValue Operations::BoolNegate(const BoolValue& bv) {
        return BoolValue(builder.buildTerm(SMTLib::Ops::Not, {bv.getTerm()}));
      }

#define X(OP)                                                                                              \
  template Value<ValType::I8> Operations::OP(const Value<ValType::I8>& l, const Value<ValType::I8>& r);    \
  template Value<ValType::I16> Operations::OP(const Value<ValType::I16>& l, const Value<ValType::I16>& r); \
  template Value<ValType::I32> Operations::OP(const Value<ValType::I32>& l, const Value<ValType::I32>& r); \
  template Value<ValType::I64> Operations::OP(const Value<ValType::I64>& l, const Value<ValType::I64>& r);

      INTOPS
#undef X
#define X(OP, SMTOP)                                                                                        \
  template Value<ValType::Bool> Operations::OP(const Value<ValType::I8>& l, const Value<ValType::I8>& r);   \
  template Value<ValType::Bool> Operations::OP(const Value<ValType::I16>& l, const Value<ValType::I16>& r); \
  template Value<ValType::Bool> Operations::OP(const Value<ValType::I32>& l, const Value<ValType::I32>& r); \
  template Value<ValType::Bool> Operations::OP(const Value<ValType::I64>& l, const Value<ValType::I64>& r);

      CMPOPS
#undef X

      template <class T>
      PointerValue Operations::PtrAdd(const PointerValue& ptrvalue, const T& addend) {
	SMTLib::Term_ptr extended;
	if constexpr (T::intbitsize () == 64) {
	  extended = addend.getTerm ();
	}
	else 
	  extended = builder.buildTerm(SMTLib::Ops::ZExt,{addend.getTerm ()},{64 - T::intbitsize ()});
	auto res = builder.buildTerm(SMTLib::Ops::BVAdd,{ptrvalue.getTerm (),extended});
	std::cerr << *res << std::endl;
	return res;
	
      }
	
      
      template <class T>
      T Operations::ExtractBaseValue(const AggregateValue& aggr, const std::size_t offset) {
	auto aggrTerm = aggr.getTerm();
	return BVHelper{builder,aggrTerm,aggr.size()}.extractBytes<NativeLoad ()> (offset,T::intbitsize () /8);
	
      }
      
      AggregateValue Operations::ExtractAggregateValue(const AggregateValue& aggr, const std::size_t offset, std::size_t size) {
	auto aggrTerm = aggr.getTerm();
        
	return {BVHelper{builder,aggrTerm,aggr.size()}.extractBytes<LoadType::Straight> (offset,size),size};

      }


      template <class T>
      AggregateValue Operations::InsertBaseValue(const AggregateValue& aggr, const std::size_t offset, const T& val) {
	return {BVHelper{builder,aggr.getTerm (),aggr.size()}.storeBytes<NativeLoad ()> (offset,val.getTerm (),T::intbitsize() / 8),aggr.size ()};
      }

      AggregateValue Operations::InsertAggregateValue(const AggregateValue& aggr, const std::size_t offset, const AggregateValue& val) {
	return {BVHelper{builder,aggr.getTerm (),aggr.size()}.storeBytes<LoadType::Straight> (offset,val.getTerm (),val.size()),aggr.size ()};
      }

      template PointerValue Operations::PtrAdd (const PointerValue&,const Value<ValType::I8>&);
      template PointerValue Operations::PtrAdd (const PointerValue&,const Value<ValType::I16>&);
      template PointerValue Operations::PtrAdd (const PointerValue&,const Value<ValType::I32>&);
      template PointerValue Operations::PtrAdd (const PointerValue&,const Value<ValType::I64>&);
      
      
      template Value<ValType::I8> Operations::ExtractBaseValue(const AggregateValue&, std::size_t);
      template Value<ValType::I16> Operations::ExtractBaseValue(const AggregateValue&, std::size_t);
      template Value<ValType::I32> Operations::ExtractBaseValue(const AggregateValue&, std::size_t);
      template Value<ValType::I64> Operations::ExtractBaseValue(const AggregateValue&, std::size_t);
      template Value<ValType::Pointer> Operations::ExtractBaseValue(const AggregateValue&, std::size_t);
      template Value<ValType::Bool> Operations::ExtractBaseValue(const AggregateValue&, std::size_t);
      
      template AggregateValue Operations::InsertBaseValue(const AggregateValue&, std::size_t, const Value<ValType::I8>&);
      template AggregateValue Operations::InsertBaseValue(const AggregateValue&, std::size_t, const Value<ValType::I16>&);
      template AggregateValue Operations::InsertBaseValue(const AggregateValue&, std::size_t, const Value<ValType::I32>&);
      template AggregateValue Operations::InsertBaseValue(const AggregateValue&, std::size_t, const Value<ValType::I64>&);

      template AggregateValue Operations::InsertBaseValue(const AggregateValue&, std::size_t, const Value<ValType::Pointer>&);
      template AggregateValue Operations::InsertBaseValue(const AggregateValue&, std::size_t, const Value<ValType::Bool>&);

    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
