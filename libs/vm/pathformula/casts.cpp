#include "vm/pathformula/operations.hpp"
#include "vm/pathformula/value.hpp"

#include "smt/builder.hpp"

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
