#include "vm/pathformula/operations.hpp"
#include "vm/pathformula/value.hpp"

#include "smt/builder.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {

      template <std::size_t bw>
      typename RetTyp<bw>::type Casts::BoolZExt(const Value<ValType::Bool>& val) {
        constexpr std::size_t bitsize = bw * 8;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(1, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }

      template <std::size_t bw>
      typename RetTyp<bw>::type Casts::BoolSExt(const Value<ValType::Bool>& val) {
        constexpr std::size_t bitsize = bw * 8;
        auto zeros = builder.makeBVIntConst(0, bitsize);
        auto ones = builder.makeBVIntConst(~0, bitsize);
        return builder.buildTerm(SMTLib::Ops::ITE, {val.getTerm(), ones, zeros});
      }

      template <class T>
      Value<ValType::Bool> Casts::IntToBool(const T& t) {
        auto tt = builder.makeBoolConst(true);
        auto ff = builder.makeBoolConst(false);
        auto zeros = builder.makeBVIntConst(0,  T::intbitsize());
        auto eq = builder.buildTerm(SMTLib::Ops::Equal, {t.getTerm(), zeros});
        return builder.buildTerm(SMTLib::Ops::ITE, {eq, ff,tt});
      }

      template <class T>
      Value<ValType::Pointer> Casts::IntToPtr(const T& t) {
	constexpr std::size_t ptrsize = Value<ValType::Pointer>::intbitsize ();
	constexpr std::size_t tsize = T::intbitsize ();
	if constexpr (ptrsize >= tsize) {
	  return builder.buildTerm (SMTLib::Ops::ZExt,{t.getTerm()},{ptrsize - tsize});
	}
	
	return builder.buildTerm (SMTLib::Ops::Extract,{t.getTerm()},{ptrsize-1,0});
      

      }

      template <size_t bw, class T>
      typename RetTyp<bw>::type Casts::Trunc (const T& t) const {
        constexpr std::size_t highbit = bw * 8 - 1;
        return builder.buildTerm(SMTLib::Ops::Extract, {t.getTerm()}, {highbit, 0});
      }

      template <size_t bw, typename T>
      typename RetTyp<bw>::type Casts::ZExt(const T& t) const {
        constexpr std::size_t bits = bw * 8 - T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::ZExt, {t.getTerm()}, {bits});
      }

      template <size_t bw, typename T>
      typename RetTyp<bw>::type Casts::SExt(const T& t) const {
        constexpr std::size_t bits = bw * 8  -T::intbitsize ();
        return builder.buildTerm(SMTLib::Ops::SExt, {t.getTerm()}, {bits});
      }


      template RetTyp<1>::type Casts::BoolZExt<1> (const Value<ValType::Bool>&);
      template RetTyp<2>::type Casts::BoolZExt<2> (const Value<ValType::Bool>&);
      template RetTyp<4>::type Casts::BoolZExt<4> (const Value<ValType::Bool>&);
      template RetTyp<8>::type Casts::BoolZExt<8> (const Value<ValType::Bool>&);

      template RetTyp<1>::type Casts::BoolSExt<1> (const Value<ValType::Bool>&);
      template RetTyp<2>::type Casts::BoolSExt<2> (const Value<ValType::Bool>&);
      template RetTyp<4>::type Casts::BoolSExt<4> (const Value<ValType::Bool>&);
      template RetTyp<8>::type Casts::BoolSExt<8> (const Value<ValType::Bool>&);
      
      template Value<ValType::Bool> Casts::IntToBool (const Value<ValType::I8>&);
      template Value<ValType::Bool> Casts::IntToBool (const Value<ValType::I16>&);
      template Value<ValType::Bool> Casts::IntToBool (const Value<ValType::I32>&);
      template Value<ValType::Bool> Casts::IntToBool (const Value<ValType::I64>&);

      template Value<ValType::Pointer> Casts::IntToPtr (const Value<ValType::I8>&);
      template Value<ValType::Pointer> Casts::IntToPtr (const Value<ValType::I16>&);
      template Value<ValType::Pointer> Casts::IntToPtr (const Value<ValType::I32>&);
      template Value<ValType::Pointer> Casts::IntToPtr (const Value<ValType::I64>&);
      
      
      template typename RetTyp<1>::type Casts::Trunc<1,Value<ValType::I8>> (const Value<ValType::I8>&) const;
      template RetTyp<1>::type Casts::Trunc<1> (const Value<ValType::I16>&) const;
      template RetTyp<2>::type Casts::Trunc<2> (const Value<ValType::I16>&) const;
      template RetTyp<1>::type Casts::Trunc<1> (const Value<ValType::I32>&) const ;
      template RetTyp<2>::type Casts::Trunc<2> (const Value<ValType::I32>&) const;
      template RetTyp<4>::type Casts::Trunc<4> (const Value<ValType::I32>&) const;
      template RetTyp<1>::type Casts::Trunc<1> (const Value<ValType::I64>&) const ;
      template RetTyp<2>::type Casts::Trunc<2> (const Value<ValType::I64>&) const;
      template RetTyp<4>::type Casts::Trunc<4> (const Value<ValType::I64>&) const;
      template RetTyp<8>::type Casts::Trunc<8> (const Value<ValType::I64>&) const;

      template RetTyp<8>::type Casts::ZExt<8> (const Value<ValType::I64>&) const;
      template RetTyp<4>::type Casts::ZExt<4> (const Value<ValType::I32>&) const;
      template RetTyp<8>::type Casts::ZExt<8> (const Value<ValType::I32>&) const;
      template RetTyp<2>::type Casts::ZExt<2> (const Value<ValType::I16>&) const;
      template RetTyp<4>::type Casts::ZExt<4> (const Value<ValType::I16>&) const;
      template RetTyp<8>::type Casts::ZExt<8> (const Value<ValType::I16>&) const ;      
      template RetTyp<1>::type Casts::ZExt<1> (const Value<ValType::I8>&) const ;
      template RetTyp<2>::type Casts::ZExt<2> (const Value<ValType::I8>&) const;
      template RetTyp<4>::type Casts::ZExt<4> (const Value<ValType::I8>&) const;
      template RetTyp<8>::type Casts::ZExt<8> (const Value<ValType::I8>&) const;

      template RetTyp<8>::type Casts::SExt<8> (const Value<ValType::I64>&) const;
      template RetTyp<4>::type Casts::SExt<4> (const Value<ValType::I32>&) const;
      template RetTyp<8>::type Casts::SExt<8> (const Value<ValType::I32>&) const;
      template RetTyp<2>::type Casts::SExt<2> (const Value<ValType::I16>&) const;
      template RetTyp<4>::type Casts::SExt<4> (const Value<ValType::I16>&) const;
      template RetTyp<8>::type Casts::SExt<8> (const Value<ValType::I16>&) const ;      
      template RetTyp<1>::type Casts::SExt<1> (const Value<ValType::I8>&) const ;
      template RetTyp<2>::type Casts::SExt<2> (const Value<ValType::I8>&) const;
      template RetTyp<4>::type Casts::SExt<4> (const Value<ValType::I8>&) const;
      template RetTyp<8>::type Casts::SExt<8> (const Value<ValType::I8>&) const;

      //template Value<ValType::Pointer> Casts::Trunc (const Value<ValType::I64>&);
      
      
    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
