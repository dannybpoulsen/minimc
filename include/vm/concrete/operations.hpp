#ifndef _VM_CONCRETE_OPS__
#define _VM_CONCRETE_OPS__

#include "support/operataions.hpp"
#include "vm/concrete/value.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      template <size_t>
      struct RetTyp;

      template <>
      struct RetTyp<1> {
        using backtype = MiniMC::uint8_t;
        using type = TValue<MiniMC::uint8_t>;
      };

      template <>
      struct RetTyp<2> {
        using backtype = MiniMC::uint16_t;
        using type = TValue<MiniMC::uint16_t>;
      };

      template <>
      struct RetTyp<4> {
        using backtype = MiniMC::uint32_t;
        using type = TValue<MiniMC::uint32_t>;
      };

      template <>
      struct RetTyp<8> {
        using backtype = MiniMC::uint64_t;
        using type = TValue<MiniMC::uint64_t>;
      };

      struct Operations {

        template <typename T>
        T Add(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::Add>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Sub(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::Sub>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Mul(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::Mul>(l.getValue(), r.getValue());
        }

        template <typename T>
        T UDiv(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::UDiv>(l.getValue(), r.getValue());
        }

        template <typename T>
        T SDiv(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::SDiv>(l.getValue(), r.getValue());
        }

        template <typename T>
        T LShl(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::Shl>(l.getValue(), r.getValue());
        }

        template <typename T>
        T LShr(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::LShr>(l.getValue(), r.getValue());
        }

        template <typename T>
        T AShr(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::AShr>(l.getValue(), r.getValue());
        }

        template <typename T>
        T And(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::And>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Or(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::Or>(l.getValue(), r.getValue());
        }

        template <typename T>
        T Xor(const T& l, const T& r) const {
          return performOp<MiniMC::Support::TAC::Xor>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue SGt(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::SGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue SGe(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::SGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue UGt(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::UGT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue UGe(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::UGE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue SLt(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::SLT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue SLe(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::SLE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue ULt(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::ULT>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue ULe(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::ULE>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue Eq(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::EQ>(l.getValue(), r.getValue());
        }

        template <typename T>
        BoolValue NEq(const T& l, const T& r) const {
          return performOp<MiniMC::Support::CMP::NEQ>(l.getValue(), r.getValue());
        }

        BoolValue PtrEq(const PointerValue& xx, const PointerValue& yy) {
          return BoolValue(yy.getValue() == xx.getValue());
        }

        template <class T>
        PointerValue PtrAdd(const PointerValue& p, const T& t) {
          return MiniMC::Support::ptradd(p.getValue(), t.getValue());
        }

        template <class T>
        auto ExtractBaseValue(const AggregateValue& value, const MiniMC::uint64_t offset) {
          if constexpr (std::is_same_v<T, TValue<MiniMC::uint8_t>>) {
            return TValue<MiniMC::uint8_t>(value.getValue().read<MiniMC::uint8_t>(offset));
          }

          else if constexpr (std::is_same_v<T, TValue<MiniMC::uint16_t>>) {
            return TValue<MiniMC::uint16_t>(value.getValue().template read<MiniMC::uint16_t>(offset));
          }

          else if constexpr (std::is_same_v<T, TValue<MiniMC::uint32_t>>) {
            return TValue<MiniMC::uint32_t>(value.getValue().template read<MiniMC::uint32_t>(offset));
          }

          else if constexpr (std::is_same_v<T, TValue<MiniMC::uint64_t>>) {
            return TValue<MiniMC::uint64_t>(value.getValue().template read<MiniMC::uint64_t>(offset));

          }

          else if constexpr (std::is_same_v<T, PointerValue>) {
            return PointerValue(value.getValue().template read<MiniMC::pointer_t>(offset));
          }
        }

        AggregateValue ExtractAggregateValue(const AggregateValue& value, const MiniMC::uint64_t offset, std::size_t size) {
          MiniMC::Util::Array extract{size};
          value.getValue().get_block(offset, size, extract.get_direct_access());
          return extract;
        }

        template <class T>
        AggregateValue InsertBaseValue(const AggregateValue& aggrvalue, const MiniMC::uint64_t offset, const T& insertee) {
          MiniMC::Util::Array arr{aggrvalue.getValue()};
          auto value = insertee.getValue();
          arr.set_block(offset, sizeof(value), reinterpret_cast<MiniMC::uint8_t*>(&value));
          return arr;
        }

	AggregateValue InsertAggregateValue(const AggregateValue& value, const MiniMC::uint64_t offset, const AggregateValue& insertee) {
          MiniMC::Util::Array arr{value.getValue()};
          arr.set_block(offset, insertee.getValue().getSize(), insertee.getValue().get_direct_access());
          return arr;
        }

	BoolValue BoolNegate (const BoolValue& negate) {
	  return negate.BoolNegate ();
	}
	
      };

      template <MiniMC::Support::TAC op, typename T>
      T performOp(T l, T r) {
        return (MiniMC::Support::Op<op>(l, r));
      }

      template <MiniMC::Support::CMP op, typename T>
      bool performOp(T l, T r) {
        return MiniMC::Support::Op<op>(l, r);
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
          MiniMC::uint64_t n = MiniMC::Support::zext<typename T::underlying_type, MiniMC::uint64_t>(t.getValue());
          return std::bit_cast<pointer_t>(n);
        }

        template <size_t bw, typename T>
        typename RetTyp<bw>::type Trunc(const T& t) const {
          using U = T::underlying_type;
          if constexpr (sizeof(U) < bw) {
            throw MiniMC::Support::Exception("Improper Truncation");
          } else if constexpr (bw == 1) {
            return TValue<MiniMC::uint8_t>(MiniMC::Support::trunc<U, MiniMC::uint8_t>(t.getValue()));
          } else if constexpr (bw == 2) {
            return TValue<MiniMC::uint16_t>(MiniMC::Support::trunc<U, MiniMC::uint16_t>(t.getValue()));
          } else if constexpr (bw == 4) {
            return TValue<MiniMC::uint32_t>(MiniMC::Support::trunc<U, MiniMC::uint32_t>(t.getValue()));
          } else if constexpr (bw == 8) {
            return TValue<MiniMC::uint64_t>(MiniMC::Support::trunc<U, MiniMC::uint64_t>(t.getValue()));
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
              return TValue<MiniMC::uint8_t>(MiniMC::Support::zext<U, MiniMC::uint8_t>(t.getValue()));
            else if constexpr (bw == 2)
              return TValue<MiniMC::uint16_t>(MiniMC::Support::zext<U, MiniMC::uint16_t>(t.getValue()));
            else if constexpr (bw == 4)
              return TValue<MiniMC::uint32_t>(MiniMC::Support::zext<U, MiniMC::uint32_t>(t.getValue()));
            else if constexpr (bw == 8)
              return TValue<MiniMC::uint64_t>(MiniMC::Support::zext<U, MiniMC::uint64_t>(t.getValue()));

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
              return TValue<MiniMC::uint8_t>(MiniMC::Support::sext<U, MiniMC::uint8_t>(t.getValue()));
            else if constexpr (bw == 2)
              return TValue<MiniMC::uint16_t>(MiniMC::Support::sext<U, MiniMC::uint16_t>(t.getValue()));
            else if constexpr (bw == 4)
              return TValue<MiniMC::uint32_t>(MiniMC::Support::sext<U, MiniMC::uint32_t>(t.getValue()));
            else if constexpr (bw == 8)
              return TValue<MiniMC::uint64_t>(MiniMC::Support::sext<U, MiniMC::uint64_t>(t.getValue()));
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
