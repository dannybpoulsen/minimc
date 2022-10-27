#include "vm/pathformula/operations.hpp"
#include "vm/pathformula/value.hpp"
#include "smt/builder.hpp"
#include "smt/smtconstruction.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      
      template<class Value>
      template <std::size_t bw>
      typename RetTyp<Value,bw>::type Casts<Value>::BoolZExt(const BoolValue& val) {
        constexpr std::size_t bitsize = bw * 8;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(1, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }

      template<class Value>
      template <std::size_t bw>
      typename RetTyp<Value,bw>::type Casts<Value>::BoolSExt(const BoolValue& val) {
        constexpr std::size_t bitsize = bw * 8;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(~0, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }


      template<class Value>
      template <class T>
      Value::Bool Casts<Value>::IntToBool(const T& t) {
        auto tt = builder.makeBoolConst(true);
        auto ff = builder.makeBoolConst(false);
        auto zeros = builder.makeBVIntConst(0,  T::intbitsize());
        auto eq = builder.buildTerm(SMTLib::Ops::Equal, {t.getTerm(), zeros});
        return builder.buildTerm(SMTLib::Ops::ITE, {eq, ff,tt});
      }

      template<class Value>
      template <class T>
      Value::Pointer Casts<Value>::IntToPtr(const T& t) {
	constexpr std::size_t ptrsize = PointerValue::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize >= tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{t.getTerm()},{ptrsize - tsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{t.getTerm()},{ptrsize-1,0});
      

      }

      template<class Value>
      template <class T>
      T Casts<Value>::PtrToInt(const typename Value::Pointer& ptr) {
	constexpr std::size_t ptrsize = Value::Pointer::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize < tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{ptr.getTerm()},{tsize - ptrsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{ptr.getTerm()},{tsize-1,0});
      

      }

      template<class Value>
      template <class T>
      T Casts<Value>::Ptr32ToInt(const typename Value::Pointer32& ptr) {
	constexpr std::size_t ptrsize =  Value::Pointer32::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize < tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{ptr.getTerm()},{tsize - ptrsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{ptr.getTerm()},{tsize-1,0});
      

      }
      
      template<class Value>
      template <class T>
      Value::Pointer32 Casts<Value>::IntToPtr32(const T& t) {
	constexpr std::size_t ptrsize = Value::Pointer32::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize >= tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{t.getTerm()},{ptrsize - tsize});
	}
	return builder.buildTerm (SMTLib::Ops::Extract,{t.getTerm()},{ptrsize-1,0});
      }

      template<class Value>
      Value::Pointer32 Casts<Value>::PtrToPtr32 (const typename Value::Pointer& p) {
	constexpr std::size_t fromsize = Value::Pointer::intbitsize ();

	using FromPtr = std::decay_t<decltype(p)>;
	
	auto constexpr segment_start = fromsize-1;
	auto constexpr segment_end = fromsize-sizeof(FromPtr::underlying_type::segment)*8;

	auto constexpr base_start = segment_end-1;
	auto constexpr base_end = segment_end-sizeof(FromPtr::underlying_type::base)*8;;

	auto constexpr offset_start = base_end-1;
	auto constexpr offset_end = base_end-sizeof(FromPtr::underlying_type::offset)*8;;
	
	
	using P32 = Value::Pointer32::underlying_type;
	
	auto term = p.getTerm ();
	auto segment = builder.buildTerm (SMTLib::Ops::Extract,{term},{segment_start,segment_end});
	auto base = builder.buildTerm (SMTLib::Ops::Extract,{term},{base_start,base_end});
	auto offset = builder.buildTerm (SMTLib::Ops::Extract,{term},{offset_start,offset_end});

	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.buildTerm (SMTLib::Ops::Extract,{segment},{sizeof(P32::segment)*8-1,0})
		<< builder.buildTerm (SMTLib::Ops::Extract,{base},{sizeof(P32::base)*8-1,0})
		<<builder.buildTerm (SMTLib::Ops::Extract,{offset},{sizeof(P32::offset)*8-1,0});
	return typename Value::Pointer32 {chainer.getTerm ()};
      }

      template<class Value>
      Value::Pointer Casts<Value>::Ptr32ToPtr (const typename Value::Pointer32& p32) {
	using FromPtr = std::decay_t<decltype(p32)>;
	
	auto term = p32.getTerm ();
	constexpr std::size_t fromsize = FromPtr::intbitsize ();
	
	
	auto constexpr segment_start = fromsize-1;
	auto constexpr segment_end = fromsize-sizeof(FromPtr::underlying_type::segment)*8;

	auto constexpr base_start = segment_end-1;
	auto constexpr base_end = segment_end-sizeof(FromPtr::underlying_type::base)*8;;

	auto constexpr offset_start = base_end-1;
	auto constexpr offset_end = base_end-sizeof(FromPtr::underlying_type::offset)*8;;
	
	
	using P = Value::Pointer::underlying_type;
	
	auto segment = builder.buildTerm (SMTLib::Ops::Extract,{term},{segment_start,segment_end});
	auto base = builder.buildTerm (SMTLib::Ops::Extract,{term},{base_start,base_end});
	auto offset = builder.buildTerm (SMTLib::Ops::Extract,{term},{offset_start,offset_end});

	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.buildTerm (SMTLib::Ops::ZExt,{segment},{(sizeof(P::segment) - sizeof(FromPtr::underlying_type::segment))*8})
		<< builder.buildTerm (SMTLib::Ops::ZExt,{base},{(sizeof(P::base) - sizeof(FromPtr::underlying_type::base))*8})
		<< builder.buildTerm (SMTLib::Ops::ZExt,{offset},{(sizeof(P::offset) - sizeof(FromPtr::underlying_type::offset))*8});
	
	return typename Value::Pointer {chainer.getTerm ()};
	
      }
      
      
      template<class Value>
      template <size_t bw, class T>
      typename RetTyp<Value,bw>::type Casts<Value>::Trunc (const T& t) const {
        constexpr std::size_t highbit = bw * 8 - 1;
        return builder.buildTerm(SMTLib::Ops::Extract, {t.getTerm()}, {highbit, 0});
      }

      template<class Value>
      template <size_t bw, typename T>
      typename RetTyp<Value,bw>::type Casts<Value>::ZExt(const T& t) const {
        constexpr std::size_t bits = bw * 8 - T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::ZExt, {t.getTerm()}, {bits});
      }

      template<class Value>
      template <size_t bw, typename T>
      typename RetTyp<Value,bw>::type Casts<Value>::SExt(const T& t) const {
        constexpr std::size_t bits = bw * 8  -T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::SExt, {t.getTerm()}, {bits});
      }

      
      template RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::BoolZExt<1> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::BoolZExt<2> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::BoolZExt<4> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::BoolZExt<8> (const PathFormulaVMVal::Bool&);

      template RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::BoolSExt<1> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::BoolSExt<2> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::BoolSExt<4> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::BoolSExt<8> (const PathFormulaVMVal::Bool&);
      
      template PathFormulaVMVal::Bool Casts<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I8> (const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Bool Casts<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I16> (const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Bool Casts<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I32> (const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Bool Casts<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I64> (const PathFormulaVMVal::I64&);

      template PathFormulaVMVal::Pointer Casts<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I8> (const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Pointer Casts<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I16> (const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Pointer Casts<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I32> (const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Pointer Casts<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I64> (const PathFormulaVMVal::I64&);
      template PathFormulaVMVal::Pointer32 Casts<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I8> (const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Pointer32 Casts<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I16> (const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Pointer32 Casts<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I32> (const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Pointer32 Casts<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I64> (const PathFormulaVMVal::I64&);

      template PathFormulaVMVal::I8 Casts<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I8> (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::I16 Casts<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I16> (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::I32 Casts<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I32> (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::I64 Casts<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I64> (const PathFormulaVMVal::Pointer&);

      template PathFormulaVMVal::I8 Casts<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I8> (const PathFormulaVMVal::Pointer32&);
      template PathFormulaVMVal::I16 Casts<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I16> (const PathFormulaVMVal::Pointer32&);
      template PathFormulaVMVal::I32 Casts<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I32> (const PathFormulaVMVal::Pointer32&);
      template PathFormulaVMVal::I64 Casts<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I64> (const PathFormulaVMVal::Pointer32&);
      
      

      
      template PathFormulaVMVal::Pointer32 Casts<PathFormulaVMVal>::PtrToPtr32 (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::Pointer Casts<PathFormulaVMVal>::Ptr32ToPtr (const PathFormulaVMVal::Pointer32&);
      
      
      template typename RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::Trunc<1,I8Value> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::Trunc<1> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::Trunc<2> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::Trunc<1> (const PathFormulaVMVal::I32&) const ;
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::Trunc<2> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::Trunc<4> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::Trunc<1> (const PathFormulaVMVal::I64&) const ;
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::Trunc<2> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::Trunc<4> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::Trunc<8> (const PathFormulaVMVal::I64&) const;

      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::ZExt<8> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::ZExt<4> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::ZExt<8> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::ZExt<2> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::ZExt<4> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::ZExt<8> (const PathFormulaVMVal::I16&) const ;      
      template RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::ZExt<1> (const PathFormulaVMVal::I8&) const ;
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::ZExt<2> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::ZExt<4> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::ZExt<8> (const PathFormulaVMVal::I8&) const;

      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::SExt<8> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::SExt<4> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::SExt<8> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::SExt<2> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::SExt<4> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::SExt<8> (const PathFormulaVMVal::I16&) const ;      
      template RetTyp<PathFormulaVMVal,1>::type Casts<PathFormulaVMVal>::SExt<1> (const PathFormulaVMVal::I8&) const ;
      template RetTyp<PathFormulaVMVal,2>::type Casts<PathFormulaVMVal>::SExt<2> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,4>::type Casts<PathFormulaVMVal>::SExt<4> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,8>::type Casts<PathFormulaVMVal>::SExt<8> (const PathFormulaVMVal::I8&) const;

      //template Value<ValType::Pointer> Casts::Trunc (const I64Value&);
      
      
    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
