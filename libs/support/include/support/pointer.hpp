#ifndef _POINTER__
#define _POINTER__

#include "support/types.hpp"

namespace MiniMC {
  namespace Support {
    
    enum class PointerType {
			    Data = 'D',
			    Location = 'L',
			    Function = 'F'
    };
    
    pointer_t makeDataPointer (base_t b,offset_t o) {
      pointer_t ptr;
      ptr.segment = static_cast<seg_t> (PointerType::Data);
      ptr.zero = 0;
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }

    pointer_t makeLocationPointer (func_t b,offset_t o) {
      pointer_t ptr;
      ptr.segment = static_cast<seg_t> (PointerType::Location);
      ptr.zero = 0;
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }


    pointer_t makeFunctionPointer (func_t b) {
      pointer_t ptr;
      ptr.segment = static_cast<seg_t> (PointerType::Function);
      ptr.zero = 0;
      ptr.base = b;
      ptr.offset = 0;
      return ptr;
    }

    
    
    template<PointerType T,PointerType... Args>
    struct IsA {
      static bool check (const pointer_t& p) {
	return p.segment == static_cast<seg_t> (T) || IsA<Args...>::check (p);
      }
    };

    template<PointerType T>
    struct IsA<T> {
      static bool check (const pointer_t& p) {
	return p.segment == static_cast<seg_t> (T);
      }
    };

      
      
    
   
    
      
    func_t getFunctionId (const pointer_t& p) {
      assert((IsA<PointerType::Data,PointerType::Function>::check (p)));
      return p.base;
    }
    
    template<typename T>
    T CastPtr (const pointer_t& ptr) {
      return MiniMC::bit_cast<pointer_struct,T> (ptr);
    }

    template<typename T>
    pointer_t CastToPtr (const T& val) {
      return MiniMC::bit_cast<T,pointer_t> (val);
    }
    
  }
}


#endif
