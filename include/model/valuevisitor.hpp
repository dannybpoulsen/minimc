#ifndef _VALUE_VISITOR__
#define _VALUE_VISITOR__

#include "model/variables.hpp"
#ifndef NDEBUG
#include "support/exceptions.hpp"
#endif

namespace MiniMC {
  namespace Model {
    
    template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };

    
    template <class F>
    auto visitValue(F f,const MiniMC::Model::Value& v) {
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
