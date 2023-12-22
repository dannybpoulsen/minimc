#include "pathvm/operations.hpp"
#include "pathvm/value.hpp"
#include "smt/builder.hpp"
#include "minimc/smt/smtconstruction.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      
      template<class Value>
      template <MiniMC::Model::TypeID to>
      typename RetTyp<Value,to>::type Operations<Value>::BoolZExt(const BoolValue& val) {
        constexpr std::size_t bitsize = MiniMC::Model::BitWidth<to>;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(1, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }

      template<class Value>
      template <MiniMC::Model::TypeID to>
      typename RetTyp<Value,to>::type Operations<Value>::BoolSExt(const BoolValue& val) {
        constexpr std::size_t bitsize = MiniMC::Model::BitWidth<to>;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(~0, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }


      template<class Value>
      template <class T>
      Value::Bool Operations<Value>::IntToBool(const T& t) {
        auto tt = builder.makeBoolConst(true);
        auto ff = builder.makeBoolConst(false);
        auto zeros = builder.makeBVIntConst(0,  T::intbitsize());
        auto eq = builder.buildTerm(SMTLib::Ops::Equal, {t.getTerm(), zeros});
        return builder.buildTerm(SMTLib::Ops::ITE, {eq, ff,tt});
      }

      template<class Value>
      template <class T>
      Value::Pointer Operations<Value>::IntToPtr(const T& t) {
	constexpr std::size_t ptrsize = PointerValue::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize >= tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{t.getTerm()},{ptrsize - tsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{t.getTerm()},{ptrsize-1,0});
      

      }

      template<class Value>
      template <class T>
      T Operations<Value>::PtrToInt(const typename Value::Pointer& ptr) {
	constexpr std::size_t ptrsize = Value::Pointer::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize < tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{ptr.getTerm()},{tsize - ptrsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{ptr.getTerm()},{tsize-1,0});
      

      }

      template<class Value>
      template <class T>
      T Operations<Value>::Ptr32ToInt(const typename Value::Pointer32& ptr) {
	constexpr std::size_t ptrsize =  Value::Pointer32::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize < tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{ptr.getTerm()},{tsize - ptrsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{ptr.getTerm()},{tsize-1,0});
      

      }
      
      template<class Value>
      template <class T>
      Value::Pointer32 Operations<Value>::IntToPtr32(const T& t) {
	constexpr std::size_t ptrsize = Value::Pointer32::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize >= tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{t.getTerm()},{ptrsize - tsize});
	}
	return builder.buildTerm (SMTLib::Ops::Extract,{t.getTerm()},{ptrsize-1,0});
      }

      template<class Value>
      Value::Pointer32 Operations<Value>::PtrToPtr32 (const typename Value::Pointer& p) {
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
      Value::Pointer Operations<Value>::Ptr32ToPtr (const typename Value::Pointer32& p32) {
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
      template <MiniMC::Model::TypeID to, class T>
      typename RetTyp<Value,to>::type Operations<Value>::Trunc (const T& t) const {
        constexpr std::size_t highbit = MiniMC::Model::BitWidth<to> - 1;
        return builder.buildTerm(SMTLib::Ops::Extract, {t.getTerm()}, {highbit, 0});
      }

      template<class Value>
      template <MiniMC::Model::TypeID to, typename T>
      typename RetTyp<Value,to>::type Operations<Value>::ZExt(const T& t) const {
        constexpr std::size_t bits = MiniMC::Model::BitWidth<to> - T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::ZExt, {t.getTerm()}, {bits});
      }

      template<class Value>
      template <MiniMC::Model::TypeID to, typename T>
      typename RetTyp<Value,to>::type Operations<Value>::SExt(const T& t) const {
        constexpr std::size_t bits = MiniMC::Model::BitWidth<to>  -T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::SExt, {t.getTerm()}, {bits});
      }

      
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::BoolZExt<MiniMC::Model::TypeID::I8> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::BoolZExt<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::BoolZExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::BoolZExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::Bool&);

      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::BoolSExt<MiniMC::Model::TypeID::I8> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::BoolSExt<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::BoolSExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::Bool&);
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::BoolSExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::Bool&);
      
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I8> (const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I16> (const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I32> (const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Bool Operations<PathFormulaVMVal>::IntToBool<PathFormulaVMVal::I64> (const PathFormulaVMVal::I64&);

      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I8> (const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I16> (const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I32> (const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::IntToPtr<PathFormulaVMVal::I64> (const PathFormulaVMVal::I64&);
      template PathFormulaVMVal::Pointer32 Operations<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I8> (const PathFormulaVMVal::I8&);
      template PathFormulaVMVal::Pointer32 Operations<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I16> (const PathFormulaVMVal::I16&);
      template PathFormulaVMVal::Pointer32 Operations<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I32> (const PathFormulaVMVal::I32&);
      template PathFormulaVMVal::Pointer32 Operations<PathFormulaVMVal>::IntToPtr32<PathFormulaVMVal::I64> (const PathFormulaVMVal::I64&);

      template PathFormulaVMVal::I8 Operations<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I8> (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::I16 Operations<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I16> (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::I32 Operations<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I32> (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::I64 Operations<PathFormulaVMVal>::PtrToInt<PathFormulaVMVal::I64> (const PathFormulaVMVal::Pointer&);

      template PathFormulaVMVal::I8 Operations<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I8> (const PathFormulaVMVal::Pointer32&);
      template PathFormulaVMVal::I16 Operations<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I16> (const PathFormulaVMVal::Pointer32&);
      template PathFormulaVMVal::I32 Operations<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I32> (const PathFormulaVMVal::Pointer32&);
      template PathFormulaVMVal::I64 Operations<PathFormulaVMVal>::Ptr32ToInt<PathFormulaVMVal::I64> (const PathFormulaVMVal::Pointer32&);
      
      

      
      template PathFormulaVMVal::Pointer32 Operations<PathFormulaVMVal>::PtrToPtr32 (const PathFormulaVMVal::Pointer&);
      template PathFormulaVMVal::Pointer Operations<PathFormulaVMVal>::Ptr32ToPtr (const PathFormulaVMVal::Pointer32&);
      
      
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I8,I8Value> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I8> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I8> (const PathFormulaVMVal::I32&) const ;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I8> (const PathFormulaVMVal::I64&) const ;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::Trunc<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I64&) const;

      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I16&) const ;      
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I8> (const PathFormulaVMVal::I8&) const ;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::ZExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I8&) const;

      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I64&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I32&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I16&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I16&) const ;      
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I8>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I8> (const PathFormulaVMVal::I8&) const ;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I16>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I16> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I32>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I32> (const PathFormulaVMVal::I8&) const;
      template RetTyp<PathFormulaVMVal,MiniMC::Model::TypeID::I64>::type Operations<PathFormulaVMVal>::SExt<MiniMC::Model::TypeID::I64> (const PathFormulaVMVal::I8&) const;

      //template Value<ValType::Pointer> Casts::Trunc (const I64Value&);
      
      
    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
