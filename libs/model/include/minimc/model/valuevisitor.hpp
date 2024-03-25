#ifndef _VALUE_VISITOR__
#define _VALUE_VISITOR__

#include "minimc/model/variables.hpp"
#ifndef NDEBUG
#include "minimc/support/exceptions.hpp"
#endif

namespace MiniMC {
  namespace Model {
    
    template<class... Ts> struct Overload : Ts... {
      using Ts::operator()...;

    };

    template<class E,class Res = void>
    concept ValueVisitator = requires(const E e,
				    const MiniMC::Model::I8Integer& i8,
				    const MiniMC::Model::I16Integer& i16,
				    const MiniMC::Model::I32Integer& i32,
				    const MiniMC::Model::I64Integer& i64,
				    const MiniMC::Model::Bool& b,
				    const MiniMC::Model::Pointer& ptr,
				    const MiniMC::Model::Pointer32& ptr32,
				    const MiniMC::Model::AggregateConstant& aggrc,
				    const MiniMC::Model::Register& reg,
				    const MiniMC::Model::Undef& und,
				    const MiniMC::Model::SymbolicConstant& sc) {
      {e(i8)}->std::convertible_to<Res>;
      {e(i16)}->std::convertible_to<Res>;
      {e(i32)}->std::convertible_to<Res>;
      {e(i64)}->std::convertible_to<Res>;
      {e(b)}->std::convertible_to<Res>;
      {e(ptr)}->std::convertible_to<Res>;
      {e(ptr32)}->std::convertible_to<Res>;
      {e(aggrc)}->std::convertible_to<Res>;
      {e(reg)}->std::convertible_to<Res>;
      {e(und)}->std::convertible_to<Res>;
      {e(sc)}->std::convertible_to<Res>;
      
    };
    
    template <class Res,ValueVisitator<Res> F>
    auto visitValue(F&& f,const MiniMC::Model::Value& v) {
      switch (v.type_t ()) {
      case ValueInfo<I8Integer>::type_t ():
	return f (static_cast<const I8Integer&> (v));
	break;
      case ValueInfo<I16Integer>::type_t ():
	return f (static_cast<const I16Integer&> (v));
	break;
      case ValueInfo<I32Integer>::type_t ():
	return f (static_cast<const I32Integer&> (v));
	break;
      case ValueInfo<I64Integer>::type_t ():
	return f (static_cast<const I64Integer&> (v));
	break;
      case ValueInfo<Bool>::type_t ():
	return f (static_cast<const Bool&> (v));
	break;
      case ValueInfo<Pointer>::type_t ():
	return f (static_cast<const Pointer&> (v));
	break;
      case ValueInfo<Pointer32>::type_t ():
	return f (static_cast<const Pointer32&> (v));
	break;
      case ValueInfo<AggregateConstant>::type_t ():
	return f (static_cast<const AggregateConstant&> (v));
	break;
      case ValueInfo<Register>::type_t ():
	return f (static_cast<const Register&> (v));
	break;
      case ValueInfo<Undef>::type_t ():
	return f (static_cast<const Undef&> (v));
	break;
      case ValueInfo<SymbolicConstant>::type_t ():
	return f (static_cast<const SymbolicConstant&> (v));
	break;
	  case ValueInfo<AddExpr>::type_t ():
	return f (static_cast<const AddExpr&> (v));
	break;
	/*case ValueInfo<SubExpr>::type_t ():
	return f (static_cast<const SubExpr&> (v));
	break;
      case ValueInfo<MulExpr>::type_t ():
	return f (static_cast<const MulExpr&> (v));
	break;
      case ValueInfo<UDivExpr>::type_t ():
	return f (static_cast<const UDivExpr&> (v));
	break;
      case ValueInfo<SDivExpr>::type_t ():
	return f (static_cast<const SDivExpr&> (v));
	break;
      case ValueInfo<LShrExpr>::type_t ():
	return f (static_cast<const LShrExpr&> (v));
	break;
      case ValueInfo<AShrExpr>::type_t ():
	return f (static_cast<const AShrExpr&> (v));
	break;
      case ValueInfo<AndExpr>::type_t ():
	return f (static_cast<const AndExpr&> (v));
	break;
      case ValueInfo<OrExpr>::type_t ():
	return f (static_cast<const OrExpr&> (v));
	break;
      case ValueInfo<XOrExpr>::type_t ():
	return f (static_cast<const XOrExpr&> (v));
	break;
      case ValueInfo<SGtExpr>::type_t ():
	return f (static_cast<const SGtExpr&> (v));
	break;
      case ValueInfo<UGtExpr>::type_t ():
	return f (static_cast<const UGtExpr&> (v));
	break;
      case ValueInfo<SGeExpr>::type_t ():
	return f (static_cast<const SGeExpr&> (v));
	break;
      case ValueInfo<UGeExpr>::type_t ():
	return f (static_cast<const UGeExpr&> (v));
	break;
      case ValueInfo<SLtExpr>::type_t ():
	return f (static_cast<const SLtExpr&> (v));
	break;
      case ValueInfo<ULtExpr>::type_t ():
	return f (static_cast<const ULtExpr&> (v));
	break;
      case ValueInfo<SLeExpr>::type_t ():
	return f (static_cast<const SLtExpr&> (v));
	break;
      case ValueInfo<ULeExpr>::type_t ():
	return f (static_cast<const ULtExpr&> (v));
	break;
      case ValueInfo<EqExpr>::type_t ():
	return f (static_cast<const SLtExpr&> (v));
	break;
      case ValueInfo<NEqExpr>::type_t ():
	return f (static_cast<const ULtExpr&> (v));
	break;*/
        default:
#ifdef NDEBUG
	__builtin_unreachable();
#else
	throw MiniMC::Support::Exception ("Missing category");
#endif
        }
      
    }

    
    
  } // namespace Model
} // namespace MiniMC

#endif
