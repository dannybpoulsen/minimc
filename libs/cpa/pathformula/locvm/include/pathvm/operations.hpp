#ifndef _VM_PATHFORMULA_OPERATIONS__
#define _VM_PATHFORMULA_OPERATIONS__

#include "pathvm/value.hpp"

#include <memory>

namespace SMTLib {
  class TermBuilder;
}

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {

      template <class Value,MiniMC::Model::TypeID>
      struct RetTyp;
      
      template<class Value>
      struct RetTyp<Value,MiniMC::Model::TypeID::I8> {
        using type = Value::I8;
	using underlying_type = MiniMC::BV8;
      };

      template<class Value>
      struct RetTyp<Value,MiniMC::Model::TypeID::I16> {
        using type = Value::I16;
	using underlying_type = MiniMC::BV16;
      };

      template<class Value>
      struct RetTyp<Value,MiniMC::Model::TypeID::I32> {
        using type = Value::I32;
	using underlying_type = MiniMC::BV32;
      };

      template<class Value>
      struct RetTyp<Value,MiniMC::Model::TypeID::I64> {
        using type = Value::I64;
	using underlying_type = MiniMC::BV64;
      };

      
      struct Operations {
	using Domain = Value;
	Operations (SMTLib::TermBuilder& builder) : builder(builder) {}
	
	template<class T>
	T Add (const T&, const T&) const;

	template<class T>
	T Sub (const T&, const T&) const;

	template<class T>
	T Mul (const T&, const T&) const;

	template<class T>
	T UDiv (const T&, const T&) const;

	template<class T>
	T SDiv (const T&, const T&)const ;

	template<class T>
	T LShl (const T&, const T&) const;

	template<class T>
	T LShr (const T&, const T&) const;

	template<class T>
	T AShr (const T&, const T&) const;

	template<class T>
	T And (const T&, const T&) const;

	template<class T>
	T Or (const T&, const T&) const;

	template<class T>
	T Xor (const T&, const T&) const;

	template<class T>
	Value::Bool SGt (const T&, const T&) const;
	
	template<class T>
	Value::Bool SGe (const T&, const T&) const;

	template<class T>
	Value::Bool SLt (const T&, const T&) const;

	template<class T>
	Value::Bool SLe (const T&, const T&) const;

	template<class T>
	Value::Bool UGt (const T&, const T&) const;

	template<class T>
	Value::Bool UGe (const T&, const T&) const;

	template<class T>
	Value::Bool ULt (const T&, const T&) const;

	template<class T>
	Value::Bool ULe (const T&, const T&) const;

	template<class T>
	Value::Bool Eq (const T&, const T&) const;

	template<class T>
	Value::Bool NEq (const T&, const T&) const;

	Value::Bool PtrEq(const PointerValue&, const PointerValue&) const {
	    throw MiniMC::Support::Exception ("Not Implemented");
	}

	Value::Bool  BoolNegate(const Value::Bool&) const;
	
        template <class T>
        Value::Pointer PtrAdd(const Value::Pointer& ptrvalue, const T& adddend) const;

	template <class T>
        Value::Pointer PtrSub(const Value::Pointer& ptrvalue, const T& adddend) const;
	
	
	template <class T>
        T ExtractBaseValue(const Value::Aggregate&, const MiniMC::BV64) const;
	
	Value::Aggregate ExtractAggregateValue(const Value::Aggregate&, const MiniMC::BV64, std::size_t) const;

	template <class T>
        Value::Aggregate InsertBaseValue(const Value::Aggregate&, const MiniMC::BV64, const T&) const;

	Value::Aggregate  InsertAggregateValue(const Value::Aggregate&, const MiniMC::BV64, const Value::Aggregate&) const;

	template <MiniMC::Model::TypeID to>
	typename RetTyp<Value,to>::type BoolZExt(const BoolValue& val) const;

	template <MiniMC::Model::TypeID to>
	typename RetTyp<Value,to>::type BoolSExt(const BoolValue& val) const;

	template <class T>
        Value::Bool IntToBool(const T& t) const;

	template <class T>
        Value::Pointer IntToPtr(const T& t) const;

	template <class T>
        Value::Pointer32 IntToPtr32(const T& t) const;
	
	
	Value::Pointer32 PtrToPtr32 (const Value::Pointer&) const;

	Value::Pointer Ptr32ToPtr (const Value::Pointer32&) const;

	template<class T>
	T PtrToInt (const Value::Pointer&) const;

	template<class T>
	T Ptr32ToInt (const Value::Pointer32&) const;
	
	
	
	template <MiniMC::Model::TypeID to, class T>
        typename RetTyp<Value,to>::type Trunc(const T& t) const;

	template <MiniMC::Model::TypeID to, typename T>
        typename RetTyp<Value,to>::type ZExt(const T& t) const;

	template <MiniMC::Model::TypeID to, typename T>
        typename RetTyp<Value,to>::type SExt(const T& t) const;


	Value create (const MiniMC::Model::I8Integer& val)  const; 
	Value create (const MiniMC::Model::I16Integer& val) const ;
	Value create (const MiniMC::Model::I32Integer& val) const ;
	Value create (const MiniMC::Model::I64Integer& val) const ;
	Value create (const MiniMC::Model::Bool& val) const   ;
	Value create (const MiniMC::Model::Pointer& val) const ;
	Value create (const MiniMC::Model::Pointer32& val) const;
	Value create (const MiniMC::Model::AggregateConstant& val) const;
	Value create (const MiniMC::Model::Undef& und) const ;
	Value create(const MiniMC::Model::SymbolicConstant& ) const    {throw MiniMC::Support::Exception ("Cannot Evaluate Symbolic Constants");}
	Value unboundValue (const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
	
	
      private:
	SMTLib::TermBuilder& builder;
      };
      
      
    }
  }
}


#endif
