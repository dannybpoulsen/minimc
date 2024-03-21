#include "pathvm/operations.hpp"
#include "pathvm/value.hpp"
#include "smt/builder.hpp"
#include "minimc/smt/smtconstruction.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      
      template <MiniMC::Model::TypeID to>
      typename RetTyp<Value,to>::type Operations::BoolZExt(const BoolValue& val) {
        constexpr std::size_t bitsize = MiniMC::Model::BitWidth<to>;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(1, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }

      template <MiniMC::Model::TypeID to>
      typename RetTyp<Value,to>::type Operations::BoolSExt(const BoolValue& val) {
        constexpr std::size_t bitsize = MiniMC::Model::BitWidth<to>;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(~0, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }


      template <class T>
      Value::Bool Operations::IntToBool(const T& t) {
        auto tt = builder.makeBoolConst(true);
        auto ff = builder.makeBoolConst(false);
        auto zeros = builder.makeBVIntConst(0,  T::intbitsize());
        auto eq = builder.buildTerm(SMTLib::Ops::Equal, {t.getTerm(), zeros});
        return builder.buildTerm(SMTLib::Ops::ITE, {eq, ff,tt});
      }

      template <class T>
      Value::Pointer Operations::IntToPtr(const T& t) {
	constexpr std::size_t ptrsize = PointerValue::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize >= tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{t.getTerm()},{ptrsize - tsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{t.getTerm()},{ptrsize-1,0});
      

      }

      template <class T>
      T Operations::PtrToInt(const typename Value::Pointer& ptr) {
	constexpr std::size_t ptrsize = Value::Pointer::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize < tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{ptr.getTerm()},{tsize - ptrsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{ptr.getTerm()},{tsize-1,0});
      

      }

      template <class T>
      T Operations::Ptr32ToInt(const typename Value::Pointer32& ptr) {
	constexpr std::size_t ptrsize =  Value::Pointer32::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize < tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{ptr.getTerm()},{tsize - ptrsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{ptr.getTerm()},{tsize-1,0});
      

      }
      
      template <class T>
      Value::Pointer32 Operations::IntToPtr32(const T& t) {
	constexpr std::size_t ptrsize = Value::Pointer32::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize >= tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{t.getTerm()},{ptrsize - tsize});
	}
	return builder.buildTerm (SMTLib::Ops::Extract,{t.getTerm()},{ptrsize-1,0});
      }

      Value::Pointer32 Operations::PtrToPtr32 (const typename Value::Pointer& p) {
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

      Value::Pointer Operations::Ptr32ToPtr (const typename Value::Pointer32& p32) {
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
      
      
      template <MiniMC::Model::TypeID to, class T>
      typename RetTyp<Value,to>::type Operations::Trunc (const T& t) const {
        constexpr std::size_t highbit = MiniMC::Model::BitWidth<to> - 1;
        return builder.buildTerm(SMTLib::Ops::Extract, {t.getTerm()}, {highbit, 0});
      }

      template <MiniMC::Model::TypeID to, typename T>
      typename RetTyp<Value,to>::type Operations::ZExt(const T& t) const {
        constexpr std::size_t bits = MiniMC::Model::BitWidth<to> - T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::ZExt, {t.getTerm()}, {bits});
      }

      template <MiniMC::Model::TypeID to, typename T>
      typename RetTyp<Value,to>::type Operations::SExt(const T& t) const {
        constexpr std::size_t bits = MiniMC::Model::BitWidth<to>  -T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::SExt, {t.getTerm()}, {bits});
      }

      
      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::BoolZExt<MiniMC::Model::TypeID::I8> (const Value::Bool&);
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::BoolZExt<MiniMC::Model::TypeID::I16> (const Value::Bool&);
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::BoolZExt<MiniMC::Model::TypeID::I32> (const Value::Bool&);
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::BoolZExt<MiniMC::Model::TypeID::I64> (const Value::Bool&);

      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::BoolSExt<MiniMC::Model::TypeID::I8> (const Value::Bool&);
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::BoolSExt<MiniMC::Model::TypeID::I16> (const Value::Bool&);
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::BoolSExt<MiniMC::Model::TypeID::I32> (const Value::Bool&);
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::BoolSExt<MiniMC::Model::TypeID::I64> (const Value::Bool&);
      
      template Value::Bool Operations::IntToBool<Value::I8> (const Value::I8&);
      template Value::Bool Operations::IntToBool<Value::I16> (const Value::I16&);
      template Value::Bool Operations::IntToBool<Value::I32> (const Value::I32&);
      template Value::Bool Operations::IntToBool<Value::I64> (const Value::I64&);

      template Value::Pointer Operations::IntToPtr<Value::I8> (const Value::I8&);
      template Value::Pointer Operations::IntToPtr<Value::I16> (const Value::I16&);
      template Value::Pointer Operations::IntToPtr<Value::I32> (const Value::I32&);
      template Value::Pointer Operations::IntToPtr<Value::I64> (const Value::I64&);
      template Value::Pointer32 Operations::IntToPtr32<Value::I8> (const Value::I8&);
      template Value::Pointer32 Operations::IntToPtr32<Value::I16> (const Value::I16&);
      template Value::Pointer32 Operations::IntToPtr32<Value::I32> (const Value::I32&);
      template Value::Pointer32 Operations::IntToPtr32<Value::I64> (const Value::I64&);

      template Value::I8 Operations::PtrToInt<Value::I8> (const Value::Pointer&);
      template Value::I16 Operations::PtrToInt<Value::I16> (const Value::Pointer&);
      template Value::I32 Operations::PtrToInt<Value::I32> (const Value::Pointer&);
      template Value::I64 Operations::PtrToInt<Value::I64> (const Value::Pointer&);

      template Value::I8 Operations::Ptr32ToInt<Value::I8> (const Value::Pointer32&);
      template Value::I16 Operations::Ptr32ToInt<Value::I16> (const Value::Pointer32&);
      template Value::I32 Operations::Ptr32ToInt<Value::I32> (const Value::Pointer32&);
      template Value::I64 Operations::Ptr32ToInt<Value::I64> (const Value::Pointer32&);
      
      

      
      
      
      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::Trunc<MiniMC::Model::TypeID::I8,I8Value> (const Value::I8&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::Trunc<MiniMC::Model::TypeID::I8> (const Value::I16&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::Trunc<MiniMC::Model::TypeID::I16> (const Value::I16&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::Trunc<MiniMC::Model::TypeID::I8> (const Value::I32&) const ;
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::Trunc<MiniMC::Model::TypeID::I16> (const Value::I32&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::Trunc<MiniMC::Model::TypeID::I32> (const Value::I32&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::Trunc<MiniMC::Model::TypeID::I8> (const Value::I64&) const ;
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::Trunc<MiniMC::Model::TypeID::I16> (const Value::I64&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::Trunc<MiniMC::Model::TypeID::I32> (const Value::I64&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::Trunc<MiniMC::Model::TypeID::I64> (const Value::I64&) const;

      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::ZExt<MiniMC::Model::TypeID::I64> (const Value::I64&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::ZExt<MiniMC::Model::TypeID::I32> (const Value::I32&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::ZExt<MiniMC::Model::TypeID::I64> (const Value::I32&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::ZExt<MiniMC::Model::TypeID::I16> (const Value::I16&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::ZExt<MiniMC::Model::TypeID::I32> (const Value::I16&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::ZExt<MiniMC::Model::TypeID::I64> (const Value::I16&) const ;      
      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::ZExt<MiniMC::Model::TypeID::I8> (const Value::I8&) const ;
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::ZExt<MiniMC::Model::TypeID::I16> (const Value::I8&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::ZExt<MiniMC::Model::TypeID::I32> (const Value::I8&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::ZExt<MiniMC::Model::TypeID::I64> (const Value::I8&) const;

      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::SExt<MiniMC::Model::TypeID::I64> (const Value::I64&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::SExt<MiniMC::Model::TypeID::I32> (const Value::I32&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::SExt<MiniMC::Model::TypeID::I64> (const Value::I32&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::SExt<MiniMC::Model::TypeID::I16> (const Value::I16&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::SExt<MiniMC::Model::TypeID::I32> (const Value::I16&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::SExt<MiniMC::Model::TypeID::I64> (const Value::I16&) const ;      
      template RetTyp<Value,MiniMC::Model::TypeID::I8>::type Operations::SExt<MiniMC::Model::TypeID::I8> (const Value::I8&) const ;
      template RetTyp<Value,MiniMC::Model::TypeID::I16>::type Operations::SExt<MiniMC::Model::TypeID::I16> (const Value::I8&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I32>::type Operations::SExt<MiniMC::Model::TypeID::I32> (const Value::I8&) const;
      template RetTyp<Value,MiniMC::Model::TypeID::I64>::type Operations::SExt<MiniMC::Model::TypeID::I64> (const Value::I8&) const;

      //template Value<ValType::Pointer> Casts::Trunc (const I64Value&);
      
      
    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
