#ifndef _VM_CONCRETE_OPS__
#define _VM_CONCRETE_OPS__

#include "host/operataions.hpp"
#include "vm/concrete/value.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      template <size_t>
      struct RetTyp;

      template <>
      struct RetTyp<1> {
        using backtype = MiniMC::BV8;
        using type = TValue<MiniMC::BV8>;
      };

      template <>
      struct RetTyp<2> {
        using backtype = MiniMC::BV16;
        using type = TValue<MiniMC::BV16>;
      };

      template <>
      struct RetTyp<4> {
        using backtype = MiniMC::BV32;
        using type = TValue<MiniMC::BV32>;
      };

      template <>
      struct RetTyp<8> {
        using backtype = MiniMC::BV64;
        using type = TValue<MiniMC::BV64>;
      };

      struct Operations {

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
        BoolValue SGt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::SGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue SGe(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::SGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue UGt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::UGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue UGe(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::UGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue SLt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::SLT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue SLe(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::SLE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue ULt(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::ULT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue ULe(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::ULE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue Eq(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::EQ>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue NEq(const T& l, const T& r) const {
          return performOp<MiniMC::Host::CMP::NEQ>(l.getValue(), r.getValue());
        }

        BoolValue PtrEq(const PointerValue& xx, const PointerValue& yy) {
          return BoolValue(yy.getValue() == xx.getValue());
        }

        template <class T>
        PointerValue PtrAdd(const PointerValue& p, const T& t) {
          return MiniMC::Support::ptradd(p.getValue(), t.getValue());
        }

        template <class T>
        auto ExtractBaseValue(const AggregateValue& value, const MiniMC::BV64 offset) {
          if constexpr (std::is_same_v<T, TValue<MiniMC::BV8>>) {
            return TValue<MiniMC::BV8>(value.getValue().read<MiniMC::BV8>(offset));
          }

          else if constexpr (std::is_same_v<T, TValue<MiniMC::BV16>>) {
            return TValue<MiniMC::BV16>(value.getValue().template read<MiniMC::BV16>(offset));
          }

          else if constexpr (std::is_same_v<T, TValue<MiniMC::BV32>>) {
            return TValue<MiniMC::BV32>(value.getValue().template read<MiniMC::BV32>(offset));
          }

          else if constexpr (std::is_same_v<T, TValue<MiniMC::BV64>>) {
            return TValue<MiniMC::BV64>(value.getValue().template read<MiniMC::BV64>(offset));

          }

          else if constexpr (std::is_same_v<T, PointerValue>) {
            return PointerValue(value.getValue().template read<MiniMC::pointer_t>(offset));
          }
        }

        AggregateValue ExtractAggregateValue(const AggregateValue& value, const MiniMC::BV64 offset, std::size_t size) {
          MiniMC::Util::Array extract{size};
          value.getValue().get_block(offset, size, extract.get_direct_access());
          return extract;
        }

        template <class T>
        AggregateValue InsertBaseValue(const AggregateValue& aggrvalue, const MiniMC::BV64 offset, const T& insertee) {
          MiniMC::Util::Array arr{aggrvalue.getValue()};
          auto value = insertee.getValue();
          arr.set_block(offset, sizeof(value), reinterpret_cast<MiniMC::BV8*>(&value));
          return arr;
        }

	AggregateValue InsertAggregateValue(const AggregateValue& value, const MiniMC::BV64 offset, const AggregateValue& insertee) {
          MiniMC::Util::Array arr{value.getValue()};
          arr.set_block(offset, insertee.getValue().getSize(), insertee.getValue().get_direct_access());
          return arr;
        }

	BoolValue BoolNegate (const BoolValue& negate) {
	  return negate.BoolNegate ();
	}
	
      };

      template <MiniMC::Host::TAC op, typename T>
      T performOp(T l, T r) {
        return (MiniMC::Host::Op<op>(l, r));
      }

      template <MiniMC::Host::CMP op, typename T>
      bool performOp(T l, T r) {
        return MiniMC::Host::Op<op>(l, r);
      }

      struct Caster {
        template <std::size_t bw>
        RetTyp<bw>::type BoolZExt(const BoolValue& val) {
          return typename RetTyp<bw>::type(val.getValue() ? 1 : 0);
        }

        template <std::size_t bw>
        RetTyp<bw>::type BoolSExt(const BoolValue& val) {
          return typename RetTyp<bw>::type(val.getValue() ? std::numeric_limits<typename RetTyp<bw>::backtype>::max() : 0);
        }

        template <class T>
        BoolValue IntToBool(const T& t) {
          return BoolValue(t.getValue());
        }

        template <class T>
        PointerValue IntToPtr(const T& t) {
          MiniMC::BV64 n = MiniMC::Host::zext<typename T::underlying_type, MiniMC::BV64>(t.getValue());
          return std::bit_cast<pointer_t>(n);
        }

        template <size_t bw, typename T>
        typename RetTyp<bw>::type Trunc(const T& t) const {
          using U = T::underlying_type;
          if constexpr (sizeof(U) < bw) {
            throw MiniMC::Support::Exception("Improper Truncation");
          } else if constexpr (bw == 1) {
            return TValue<MiniMC::BV8>(MiniMC::Host::trunc<U, MiniMC::BV8>(t.getValue()));
          } else if constexpr (bw == 2) {
            return TValue<MiniMC::BV16>(MiniMC::Host::trunc<U, MiniMC::BV16>(t.getValue()));
          } else if constexpr (bw == 4) {
            return TValue<MiniMC::BV32>(MiniMC::Host::trunc<U, MiniMC::BV32>(t.getValue()));
          } else if constexpr (bw == 8) {
            return TValue<MiniMC::BV64>(MiniMC::Host::trunc<U, MiniMC::BV64>(t.getValue()));
          } else {
            []<bool t = false>() { static_assert(t); }
            ();
          }
        }

        template <size_t bw, typename T>
        typename RetTyp<bw>::type ZExt(const T& t) const {
          using U = T::underlying_type;
          if constexpr (sizeof(U) > bw) {
            throw MiniMC::Support::Exception("Improper Extention");
          } else {
            if constexpr (bw == 1)
              return TValue<MiniMC::BV8>(MiniMC::Host::zext<U, MiniMC::BV8>(t.getValue()));
            else if constexpr (bw == 2)
              return TValue<MiniMC::BV16>(MiniMC::Host::zext<U, MiniMC::BV16>(t.getValue()));
            else if constexpr (bw == 4)
              return TValue<MiniMC::BV32>(MiniMC::Host::zext<U, MiniMC::BV32>(t.getValue()));
            else if constexpr (bw == 8)
              return TValue<MiniMC::BV64>(MiniMC::Host::zext<U, MiniMC::BV64>(t.getValue()));

            else {
              []<bool t = false>() { static_assert(t); }
              ();
            }
          }
        }

        template <size_t bw, typename T>
        typename RetTyp<bw>::type SExt(const T& t) const {
          using U = T::underlying_type;
          if constexpr (sizeof(T) > bw) {
            throw MiniMC::Support::Exception("Improper Extention");
          } else {
            if constexpr (bw == 1)
              return TValue<MiniMC::BV8>(MiniMC::Host::sext<U, MiniMC::BV8>(t.getValue()));
            else if constexpr (bw == 2)
              return TValue<MiniMC::BV16>(MiniMC::Host::sext<U, MiniMC::BV16>(t.getValue()));
            else if constexpr (bw == 4)
              return TValue<MiniMC::BV32>(MiniMC::Host::sext<U, MiniMC::BV32>(t.getValue()));
            else if constexpr (bw == 8)
              return TValue<MiniMC::BV64>(MiniMC::Host::sext<U, MiniMC::BV64>(t.getValue()));
            else {
              []<bool t = false>() { static_assert(t); }
              ();
            }
          }
        }
      };

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC

#endif
