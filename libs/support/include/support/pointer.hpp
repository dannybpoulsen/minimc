#ifndef _POINTER__
#define _POINTER__

#include "support/types.hpp"

namespace MiniMC {
  namespace Support {
    
    enum class PointerType {
			    Stack = 'D',
			    Heap = 'H',
			    Location = 'L',
			    Function = 'F'
    };
    
    inline pointer_t makeStackPointer (proc_t b,offset_t o) {
      pointer_t ptr;
      ptr.segment = static_cast<seg_t> (PointerType::Stack);
      ptr.zero = 0;
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }

    inline pointer_t makeHeapPointer (base_t b,offset_t o) {
      pointer_t ptr;
      ptr.segment = static_cast<seg_t> (PointerType::Heap);
      ptr.zero = 0;
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }
    
    inline pointer_t makeLocationPointer (func_t b,offset_t o) {
      pointer_t ptr;
      ptr.segment = static_cast<seg_t> (PointerType::Location);
      ptr.zero = 0;
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }


    inline pointer_t makeFunctionPointer (func_t b) {
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

      
      
    
   
    
      
    inline func_t getFunctionId (const pointer_t& p) {
      assert((IsA<PointerType::Location,PointerType::Function>::check (p)));
      return p.base;
    }

    inline base_t getBase (const pointer_t& p) {
      assert((IsA<PointerType::Heap>::check (p)));
      return p.base;
    }

    inline base_t getOffset (const pointer_t& p) {
      assert((IsA<PointerType::Heap>::check (p)));
      return p.offset;
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
