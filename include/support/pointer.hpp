#ifndef _POINTER__
#define _POINTER__
#include <cassert>
#include <cstdlib>

#include "host/types.hpp"

namespace MiniMC {
  namespace Support {

    enum class PointerType {
      Stack = 'D',
      Heap = 'H',
      Location = 'L',
      Function = 'F'
    };

    
    inline pointer_t makeStackPointer(proc_t b, offset_t o) {
      pointer_t ptr{};
      ptr.segment = static_cast<seg_t>(PointerType::Stack);
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }

    inline pointer_t makeHeapPointer(base_t b, offset_t o) {
      pointer_t ptr{};
      ptr.segment = static_cast<seg_t>(PointerType::Heap);
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }

    inline pointer_t makeLocationPointer(func_t b, offset_t o) {
      pointer_t ptr{};
      ptr.segment = static_cast<seg_t>(PointerType::Location);
      ptr.base = b;
      ptr.offset = o;
      return ptr;
    }

    inline pointer_t makeFunctionPointer(func_t b) {
      pointer_t ptr{};
      ptr.segment = static_cast<seg_t>(PointerType::Function);
      ptr.base = b;
      ptr.offset = 0;
      return ptr;
    }
    
    

    inline func_t getFunctionId(const pointer_t& p) {
      return p.base;
    }

    inline base_t getBase(const pointer_t& p) {
      return p.base;
    }

    inline offset_t getOffset(const pointer_t& p) {
      return p.offset;
    }
    
    
  } // namespace Support
} // namespace MiniMC

#endif
