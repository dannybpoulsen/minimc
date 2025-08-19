#ifndef _VM_CONCRETE_OPS__
#define _VM_CONCRETE_OPS__

#include "minimc/host/operataions.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/values/concrete/value.hpp"

#include <span>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      template <MiniMC::Model::TypeID>
      struct RetTyp;

      template <>
      struct RetTyp<MiniMC::Model::TypeID::I8> {
        using backtype = MiniMC::BV8;
        using type = TValue<MiniMC::BV8>;
      };

      template <>
      struct RetTyp<MiniMC::Model::TypeID::I16> {
        using backtype = MiniMC::BV16;
        using type = TValue<MiniMC::BV16>;
      };

      template <>
      struct RetTyp<MiniMC::Model::TypeID::I32> {
        using backtype = MiniMC::BV32;
        using type = TValue<MiniMC::BV32>;
      };

      template <>
      struct RetTyp<MiniMC::Model::TypeID::I64> {
        using backtype = MiniMC::BV64;
        using type = TValue<MiniMC::BV64>;
      };
      
      template <MiniMC::Host::TAC op, typename T>
      T performOp(T l, T r) {
        return (MiniMC::Host::Op<op>(l, r));
      }
      
      template <MiniMC::Host::CMP op, typename T>
      bool performOp(T l, T r) {
        return MiniMC::Host::Op<op>(l, r);
      }

      struct Operations {
	using Domain = Value;
	template <typename T>
        T Not(const T& l) const requires Integer<Value,T> {
          return ~l.getValue();
        }

	template <typename T>
        T Add(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::TAC::Add>(l.getValue(), r.getValue());
        }
	
        template <typename T>
        T Sub(const T& l, const T& r) const requires Integer<Value,T>{
          return performOp<MiniMC::Host::TAC::Sub>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Mul(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::TAC::Mul>(l.getValue(), r.getValue());
        }

        template <typename T>
        T UDiv(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::TAC::UDiv>(l.getValue(), r.getValue());
        }

        template <typename T>
        T SDiv(const T& l, const T& r) const requires Integer<Value,T>{
          return performOp<MiniMC::Host::TAC::SDiv>(l.getValue(), r.getValue());
        }

        template <typename T>
        T LShl(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::TAC::LShl>(l.getValue(), r.getValue());
        }

        template <typename T>
        T LShr(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::TAC::LShr>(l.getValue(), r.getValue());
        }

        template <typename T>
        T AShr(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::TAC::AShr>(l.getValue(), r.getValue());
        }

        template <typename T>
        T And(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::TAC::And>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Or(const T& l, const T& r) const requires Integer<Value,T>{
          return performOp<MiniMC::Host::TAC::Or>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Xor(const T& l, const T& r) const requires Integer<Value,T>{
          return performOp<MiniMC::Host::TAC::Xor>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SGt(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::CMP::SGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SGe(const T& l, const T& r) const requires Integer<Value,T>{
	  return performOp<MiniMC::Host::CMP::SGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool UGt(const T& l, const T& r) const requires Integer<Value,T>{
          return performOp<MiniMC::Host::CMP::UGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool UGe(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::CMP::UGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SLt(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::CMP::SLT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SLe(const T& l, const T& r) const requires Integer<Value,T>{
          return performOp<MiniMC::Host::CMP::SLE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool ULt(const T& l, const T& r) const  requires Integer<Value,T> {
          return performOp<MiniMC::Host::CMP::ULT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool ULe(const T& l, const T& r) const requires Integer<Value,T>{
          return performOp<MiniMC::Host::CMP::ULE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool Eq(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::CMP::EQ>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool NEq(const T& l, const T& r) const requires Integer<Value,T> {
          return performOp<MiniMC::Host::CMP::NEQ>(l.getValue(), r.getValue());
        }

        
        template <class T>
        Value::Pointer PtrAdd(const Value::Pointer& p, const T& t) const {
          return p.getValue().add( t.getValue());
        }

	template <class T>
        Value::Pointer PtrSub(const Value::Pointer& p, const T& t) const {
          return p.getValue().sub ( t.getValue());
        }
	
        template <class T>
        T ExtractBaseValue(const Value::Aggregate& value, const MiniMC::BV64 offset) const  {
	  return value.getValue().template read<typename T::underlying_type> (offset);
	}

	Value::Aggregate ExtractAggregateValue(const Value::Aggregate& value, const MiniMC::BV64 offset, std::size_t size) const  {
          return value.getValue().get_block(offset, size);
        }

        template <class T>
        Value::Aggregate InsertBaseValue(const Value::Aggregate& aggrvalue, const MiniMC::BV64 offset, const T& insertee)  const requires (!MiniMC::VMT::MemoryC<Value,T>) {
          MiniMC::Util::Array arr{aggrvalue.getValue()};
          auto value = insertee.getValue();
          arr.set_block(offset, {reinterpret_cast<MiniMC::BV8*>(&value),reinterpret_cast<MiniMC::BV8*>(&value)+sizeof(value)});
          return arr;
        }
	
	Value::Aggregate InsertAggregateValue(const Value::Aggregate& value, const MiniMC::BV64 offset, const AggregateValue& insertee) const {
          MiniMC::Util::Array arr{value.getValue()};
          arr.set_block(offset, insertee.getValue().get_direct_access());
          return arr;
        }

	Value::Bool BoolNegate (const Value::Bool& negate) const  {
	  return negate.BoolNegate ();
	}
	
	
	
	template <MiniMC::Model::TypeID to>
        RetTyp<to>::type ZExt(const BoolValue& val) const  {
          return typename RetTyp<to>::type(val.getValue() ? 1 : 0);
        }

        template <MiniMC::Model::TypeID to>
        RetTyp<to>::type SExt(const BoolValue& val)  const {
          return typename RetTyp<to>::type(val.getValue() ? std::numeric_limits<typename RetTyp<to>::backtype>::max() : 0);
        }

        template <class T>
        Value::Bool IntToBool(const T& t)  const {
          return BoolValue(t.getValue());
        }

        
	
	Value::Pointer32 PtrToPtr32 (const Value::Pointer& p ) const  {
	  auto base = static_cast<Value::Pointer32::underlying_type::BaseT>(p.getValue().base);
	  auto offset = static_cast<Value::Pointer32::underlying_type::OffsetT>(p.getValue().offset);
							     auto segment = static_cast<Value::Pointer32::underlying_type::SegT>(p.getValue().segment);
	  
							     return Value::Pointer32::underlying_type{segment,base,offset};
	}

	Value::Pointer Ptr32ToPtr (const Value::Pointer32& p32) const  {
	  auto base = p32.getValue().base;
	  auto offset = p32.getValue().offset;
	  auto segment = p32.getValue().segment;
	  return Value::Pointer::underlying_type{segment,base,offset};
	}

	template<typename To,typename From>
	To  BitCast (const From& val) const requires (Integer<Value,To> && (Pointer<Value,From>) && sizeof(typename To::underlying_type) == sizeof(typename From::underlying_type)) {
	  auto value = val.getValue();
	  return {std::bit_cast<typename To::underlying_type> (value)};
	};
	
	template<typename To,typename From>
	To  BitCast (const From& val) const requires (Aggregate<Value,To> && (Integer<Value,From> || Pointer<Value,From>)) {
	  auto value = val.getValue();
	  return MiniMC::Util::Array {sizeof(value),reinterpret_cast<MiniMC::BV8*>(&value)};
	};

	
	
	template<typename To,typename From>
	To  BitCast (const From& val) const requires (Pointer<Value,To> && Integer<Value,From> && sizeof(typename To::underlying_type) == sizeof(typename From::underlying_type)) {
	  auto value = val.getValue();
	  return {std::bit_cast<typename To::underlying_type> (value)};
	}

	template<typename To>
	To  BitCast (const Value::Aggregate& val) const requires (!Aggregate<Value,To>) {
	  auto value = val.getValue();
	  if (value.getSize () != sizeof(To::underlying_type)) {
	    throw MiniMC::Support::Exception {"Impropert bitcast"};
	  }
	  typename To::underlying_type res;
	  std::copy(value.get_direct_access(),&res);
	  return {std::bit_cast<typename To::underlying_type> (value)};
	}
	
        template <MiniMC::Model::TypeID to, typename T>
        typename RetTyp<to>::type Trunc(const T& t) const {
          using U = T::underlying_type;
          if constexpr (sizeof(U)*8 < MiniMC::Model::BitWidth<to>) {
            throw MiniMC::Support::Exception("Improper Truncation");
          } else if constexpr (to == MiniMC::Model::TypeID::I8) {
            return TValue<MiniMC::BV8>(MiniMC::Host::trunc<U, MiniMC::BV8>(t.getValue()));
          } else if constexpr (to == MiniMC::Model::TypeID::I16) {
            return TValue<MiniMC::BV16>(MiniMC::Host::trunc<U, MiniMC::BV16>(t.getValue()));
          } else if constexpr (to ==  MiniMC::Model::TypeID::I32) {
            return TValue<MiniMC::BV32>(MiniMC::Host::trunc<U, MiniMC::BV32>(t.getValue()));
          } else if constexpr (to == MiniMC::Model::TypeID::I64) {
            return TValue<MiniMC::BV64>(MiniMC::Host::trunc<U, MiniMC::BV64>(t.getValue()));
          } else {
            []<bool t = false>() { static_assert(t); }
            ();
          }
        }

	Value::Bool BoolAnd(const Value::Bool& l, const Value::Bool& r ) const {
	  return Value::Bool (l.getValue() && r.getValue());
	}
	
        template <MiniMC::Model::TypeID to, typename T>
        typename RetTyp<to>::type ZExt(const T& t) const {
          using U = T::underlying_type;
          if constexpr (sizeof(U) > MiniMC::Model::BitWidth<to>) {
            throw MiniMC::Support::Exception("Improper Extention");
          } else {
            if constexpr (to == MiniMC::Model::TypeID::I8)
              return TValue<MiniMC::BV8>(MiniMC::Host::zext<U, MiniMC::BV8>(t.getValue()));
            else if constexpr (to == MiniMC::Model::TypeID::I16)
              return TValue<MiniMC::BV16>(MiniMC::Host::zext<U, MiniMC::BV16>(t.getValue()));
            else if constexpr (to == MiniMC::Model::TypeID::I32)
              return TValue<MiniMC::BV32>(MiniMC::Host::zext<U, MiniMC::BV32>(t.getValue()));
            else if constexpr (to == MiniMC::Model::TypeID::I64)
              return TValue<MiniMC::BV64>(MiniMC::Host::zext<U, MiniMC::BV64>(t.getValue()));

            else {
              []<bool t = false>() { static_assert(t); }
              ();
            }
          }
        }

        template <MiniMC::Model::TypeID to, typename T>
        typename RetTyp<to>::type SExt(const T& t) const {
          using U = T::underlying_type;
          if constexpr (sizeof(T) > MiniMC::Model::BitWidth<to>) {
            throw MiniMC::Support::Exception("Improper Extention");
          } else {
            if constexpr (to == MiniMC::Model::TypeID::I8)
              return TValue<MiniMC::BV8>(MiniMC::Host::sext<U, MiniMC::BV8>(t.getValue()));
            else if constexpr (to == MiniMC::Model::TypeID::I16)
              return TValue<MiniMC::BV16>(MiniMC::Host::sext<U, MiniMC::BV16>(t.getValue()));
            else if constexpr (to == MiniMC::Model::TypeID::I32)
              return TValue<MiniMC::BV32>(MiniMC::Host::sext<U, MiniMC::BV32>(t.getValue()));
            else if constexpr (to == MiniMC::Model::TypeID::I64)
              return TValue<MiniMC::BV64>(MiniMC::Host::sext<U, MiniMC::BV64>(t.getValue()));
            else {
              []<bool t = false>() { static_assert(t); }
              ();
            }
          }
        }

	template<class T>
	std::generator<Value::I8> bytes(const T& l) const requires (Integer<Value,T> || Pointer<Value,T>) 
	{
	  auto val = l.getValue();
	  std::span<MiniMC::BV8> mspan {reinterpret_cast<MiniMC::BV8*> (&val),sizeof(val)};
	  for (auto t : mspan) {
	    co_yield Value::I8 (t);
	  }
	  
	}

	std::generator<Value::I8> bytes(const Value::Aggregate& l)  const 
	{
	  auto val = l.getValue();
	  for (auto t : val.get_direct_access()) {
	    co_yield Value::I8 (t);
	  }
	  
	}

	std::generator<Value::I8> extractbytes(const Value::Aggregate& l,std::size_t offset, std::size_t bytes)  const 
	{
	  if (offset +bytes >=l.getValue().getSize()) {
	    throw MiniMC::Support::Exception {"Extracting too many bytes from aggregate"};
	  }
	  for (auto b : l.getValue().get_direct_access ().subspan (offset,bytes)) {
	    co_yield Value::I8 {b};
	  }
	  
	}
	
	Value::I8 create (const MiniMC::Model::I8Integer& val)  const  { return Value::I8{val.getValue()}; }
	Value::I16 create (const MiniMC::Model::I16Integer& val) const { return Value::I16{val.getValue()}; }
	Value::I32 create (const MiniMC::Model::I32Integer& val) const  { return Value::I32{val.getValue()}; }
	Value::I64 create (const MiniMC::Model::I64Integer& val) const  { return Value::I64{val.getValue()}; }
	Value::Bool create (const MiniMC::Model::Bool& val) const   { return Value::Bool{val.getValue()}; }
	Value::Pointer create (const MiniMC::Model::Pointer& val) const  { return Value::Pointer{val.getValue()}; }
	Value::Pointer32 create (const MiniMC::Model::Pointer32& val) const   { return Value::Pointer32{val.getValue()}; }
	Value::Aggregate create (const MiniMC::Model::AggregateConstant& val) const   {return AggregateValue(val.getData());}
	std::generator<Value> create (const MiniMC::Model::Undef& und) const;
	//Value unboundValue (const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
	
	
      };
      
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC

#endif
