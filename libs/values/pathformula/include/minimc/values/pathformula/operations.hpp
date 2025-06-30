#ifndef _VM_PATHFORMULA_OPERATIONS__
#define _VM_PATHFORMULA_OPERATIONS__

#include "minimc/values/pathformula/value.hpp"

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
	T Not (const T&) const;
	
	
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
	typename RetTyp<Value,to>::type ZExt(const BoolValue& val) const;

	template <MiniMC::Model::TypeID to>
	typename RetTyp<Value,to>::type SExt(const BoolValue& val) const;
	
	template <class T>
        Value::Bool IntToBool(const T& t) const;
	
	
	Value::Pointer32 PtrToPtr32 (const Value::Pointer&) const;

	Value::Pointer Ptr32ToPtr (const Value::Pointer32&) const;
	
	Value::Bool BoolAnd (const Value::Bool&, const Value::Bool&) const; 
	
	
	template<typename To,typename From>
	To  BitCast (const From& val) const   {
	  auto term = val.getTerm();
	  return {std::move(term)};
	}

	template<typename To,typename From>
	To  BitCast (const From& val) const requires Aggregate<Value,To>  {
	  auto term = val.getTerm();
	  return {std::move(term),val.size()};
	}

	std::generator<Value::I8> bytes (const Value::I8&) const; 
	std::generator<Value::I8> bytes (const Value::I16&) const;
	std::generator<Value::I8> bytes (const Value::I32&) const;
	std::generator<Value::I8> bytes (const Value::I64&) const;
	std::generator<Value::I8> bytes (const Value::Pointer&) const;
	std::generator<Value::I8> bytes (const Value::Pointer32&) const;
	std::generator<Value::I8> bytes (const Value::Aggregate&) const;
	std::generator<Value::I8> extractbytes(const Value::Aggregate& l,std::size_t offset, std::size_t bytes)  const;
	
	
	template <MiniMC::Model::TypeID to, class T>
        typename RetTyp<Value,to>::type Trunc(const T& t) const;

	template <MiniMC::Model::TypeID to, typename T>
        typename RetTyp<Value,to>::type ZExt(const T& t) const requires MiniMC::VMT::Integer<Value,T> ;

	template <MiniMC::Model::TypeID to, typename T>
        typename RetTyp<Value,to>::type SExt(const T& t) const requires MiniMC::VMT::Integer<Value,T> ;;


	Value::I8 create (const MiniMC::Model::I8Integer& val)  const; 
	Value::I16 create (const MiniMC::Model::I16Integer& val) const ;
	Value::I32 create (const MiniMC::Model::I32Integer& val) const ;
	Value::I64 create (const MiniMC::Model::I64Integer& val) const ;
	Value::Bool create (const MiniMC::Model::Bool& val) const   ;
	Value::Pointer create (const MiniMC::Model::Pointer& val) const ;
	Value::Pointer32 create (const MiniMC::Model::Pointer32& val) const;
	Value::Aggregate create (const MiniMC::Model::AggregateConstant& val) const;
	std::generator<Value>create (const MiniMC::Model::Undef& und) const ;
	Value create(const MiniMC::Model::SymbolicConstant& ) const    {throw MiniMC::Support::Exception ("Cannot Evaluate Symbolic Constants");}
	Value unboundValue (const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
	

	
      private:
	SMTLib::TermBuilder& builder;
      };
      
      
      class Memory  {
      public:
	Memory (SMTLib::TermBuilder& b);
	Memory (const Memory&) = default;
	Memory& operator= (Memory&& m) = default;
	std::generator<Value::I8> loadBytes(const MemoryValue&, const typename Value::Pointer&, std::size_t) const ;
        
	// First parameter is address to store at, second is the value to state
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I8&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::I16&) const ;
        MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::I32&) const ;
        MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::I64&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::Aggregate&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::Pointer&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::Pointer32&) const ;
	
	// PArameter is size to allocate
	MemoryValue allocate(const MemoryValue&,const Value::Pointer&, const Value::I64&) ;
	Value::Pointer  find_space(const MemoryValue&,const Value::I64&) ;
	
	
        MemoryValue free(const MemoryValue& m, const Value::Pointer&)  {return m;}
      private:
	SMTLib::TermBuilder* builder;
      };
      
      
    }
  }
}


#endif
