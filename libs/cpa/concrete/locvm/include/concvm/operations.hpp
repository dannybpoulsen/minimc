#ifndef _VM_CONCRETE_OPS__
#define _VM_CONCRETE_OPS__

#include "minimc/host/operataions.hpp"
#include "concvm/value.hpp"

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
        T Add(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::Add>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Sub(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::Sub>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Mul(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::Mul>(l.getValue(), r.getValue());
        }

        template <typename T>
        T UDiv(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::UDiv>(l.getValue(), r.getValue());
        }

        template <typename T>
        T SDiv(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::SDiv>(l.getValue(), r.getValue());
        }

        template <typename T>
        T LShl(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::Shl>(l.getValue(), r.getValue());
        }

        template <typename T>
        T LShr(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::LShr>(l.getValue(), r.getValue());
        }

        template <typename T>
        T AShr(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::AShr>(l.getValue(), r.getValue());
        }

        template <typename T>
        T And(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::And>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Or(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::Or>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Xor(const T& l, const T& r) const {
          return performOp<MiniMC::Host::TAC::Xor>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SGt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::SGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SGe(const T& l, const T& r) const {
	  return performOp<MiniMC::Host::CMP::SGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool UGt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::UGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool UGe(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::UGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SLt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::SLT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool SLe(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::SLE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool ULt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::ULT>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool ULe(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::ULE>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool Eq(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::EQ>(l.getValue(), r.getValue());
        }

        template <typename T>
        Value::Bool NEq(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::NEQ>(l.getValue(), r.getValue());
        }

        Value::Bool PtrEq(const Value::Pointer& xx, const Value::Pointer& yy) {
          return typename Value::Bool(yy.getValue() == xx.getValue());
        }

        template <class T>
        Value::Pointer PtrAdd(const Value::Pointer& p, const T& t) {
          return p.getValue().add( t.getValue());
        }

	template <class T>
        Value::Pointer PtrSub(const Value::Pointer& p, const T& t) {
          return p.getValue().sub ( t.getValue());
        }
	
        template <class T>
        T ExtractBaseValue(const Value::Aggregate& value, const MiniMC::BV64 offset) {
	  return value.getValue().template read<typename T::underlying_type> (offset);
	}

	Value::Aggregate ExtractAggregateValue(const Value::Aggregate& value, const MiniMC::BV64 offset, std::size_t size) {
          return value.getValue().get_block(offset, size);
        }

        template <class T>
        Value::Aggregate InsertBaseValue(const Value::Aggregate& aggrvalue, const MiniMC::BV64 offset, const T& insertee) {
          MiniMC::Util::Array arr{aggrvalue.getValue()};
          auto value = insertee.getValue();
          arr.set_block(offset, sizeof(value), reinterpret_cast<MiniMC::BV8*>(&value));
          return arr;
        }
	
	Value::Aggregate InsertAggregateValue(const Value::Aggregate& value, const MiniMC::BV64 offset, const AggregateValue& insertee) {
          MiniMC::Util::Array arr{value.getValue()};
          arr.set_block(offset, insertee.getValue().getSize(), insertee.getValue().get_direct_access());
          return arr;
        }

	Value::Bool BoolNegate (const Value::Bool& negate) {
	  return negate.BoolNegate ();
	}

	        template <MiniMC::Model::TypeID to>
        RetTyp<to>::type BoolZExt(const BoolValue& val) {
          return typename RetTyp<to>::type(val.getValue() ? 1 : 0);
        }

        template <MiniMC::Model::TypeID to>
        RetTyp<to>::type BoolSExt(const BoolValue& val) {
          return typename RetTyp<to>::type(val.getValue() ? std::numeric_limits<typename RetTyp<to>::backtype>::max() : 0);
        }

        template <class T>
        Value::Bool IntToBool(const T& t) {
          return BoolValue(t.getValue());
        }

        template <class T>
        Value::Pointer IntToPtr(const T& t) {
	  Value::Pointer::underlying_type::PtrBV n;
	  if constexpr (sizeof(typename T::underlying_type) <= sizeof(decltype(n))) {
	    n = MiniMC::Host::zext<typename T::underlying_type, decltype(n)>(t.getValue());
	  }
	  else {
	    n = MiniMC::Host::trunc<typename T::underlying_type, decltype(n)>(t.getValue());
	  }
	  return std::bit_cast<PointerValue::underlying_type>(n);
	}

	template <class T>
        T PtrToInt(const Value::Pointer& t) {
	  typename T::underlying_type n;
	  auto ptrval = std::bit_cast<Value::Pointer::underlying_type::PtrBV> (t.getValue());
	  if constexpr (sizeof(typename T::underlying_type) >= sizeof(decltype(n))) {
	    n = MiniMC::Host::zext<decltype(n),typename T::underlying_type>(ptrval);
	  }
	  else {
	    n = MiniMC::Host::trunc<decltype(ptrval), decltype(n)>(ptrval);
	  }
	  return T{n};
	}

	template <class T>
        T Ptr32ToInt(const Value::Pointer32& t) {
	  typename T::underlying_type n;
	  auto ptrval = std::bit_cast<Value::Pointer32::underlying_type::PtrBV> (t.getValue());
	  if constexpr (sizeof(typename T::underlying_type) >= sizeof(decltype(n))) {
	    n = MiniMC::Host::zext<decltype(n),typename T::underlying_type>(ptrval);
	  }
	  else {
	    n = MiniMC::Host::trunc<decltype(ptrval), decltype(n)>(ptrval);
	  }
	  return T{n};
	}
	
	template <class T>
        Value::Pointer32 IntToPtr32(const T& t) {
	  Value::Pointer32::underlying_type::PtrBV n;
	  if constexpr (sizeof(typename T::underlying_type) <= sizeof(decltype(n))) {
	    n = MiniMC::Host::zext<typename T::underlying_type, decltype(n)>(t.getValue());
	  }
	  else {
	    n = MiniMC::Host::trunc<typename T::underlying_type, decltype(n)>(t.getValue());
	  }
	  return std::bit_cast<Value::Pointer32::underlying_type>(n);
	}
	
	
	Value::Pointer32 PtrToPtr32 (const Value::Pointer& p ) {
	  Value::Pointer32::underlying_type p32{};
	  p32.base = p.getValue().base;
	  p32.offset = p.getValue().offset;
	  p32.segment = p.getValue().segment;
	  
	  return p32;
	}

	Value::Pointer Ptr32ToPtr (const Value::Pointer32& p32) {
	  Value::Pointer::underlying_type p{};
	  p.base = p32.getValue().base;
	  p.offset = p32.getValue().offset;
	  p.segment = p32.getValue().segment;
	  return p;
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

	Value create (const MiniMC::Model::I8Integer& val)  const  { return Value::I8{val.getValue()}; }
	Value create (const MiniMC::Model::I16Integer& val) const { return Value::I16{val.getValue()}; }
	Value create (const MiniMC::Model::I32Integer& val) const  { return Value::I32{val.getValue()}; }
	Value create (const MiniMC::Model::I64Integer& val) const  { return Value::I64{val.getValue()}; }
	Value create (const MiniMC::Model::Bool& val) const   { return Value::Bool{val.getValue()}; }
	Value create (const MiniMC::Model::Pointer& val) const  { return Value::Pointer{val.getValue()}; }
	Value create (const MiniMC::Model::Pointer32& val) const   { return Value::Pointer32{val.getValue()}; }
	Value create (const MiniMC::Model::AggregateConstant& val) const   {return AggregateValue(val.getData());}
	Value create (const MiniMC::Model::Undef& und) const { return this->unboundValue (*und.getType ()); }
	Value create(const MiniMC::Model::SymbolicConstant& ) const    {throw MiniMC::Support::Exception ("Cannot Evaluate Symbolic Constants");}
	Value unboundValue (const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
	
	
      };

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC

#endif
