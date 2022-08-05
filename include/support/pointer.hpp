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

    template <PointerType T, PointerType... Args>
    struct IsA {
      static bool check(const pointer_t& p) {
        return p.segment == static_cast<seg_t>(T) || IsA<Args...>::check(p);
      }
    };

    template <PointerType T>
    struct IsA<T> {
      static bool check(const pointer_t& p) {
        return p.segment == static_cast<seg_t>(T);
      }
    };

    inline func_t getFunctionId(const pointer_t& p) {
      assert((IsA<PointerType::Location, PointerType::Function>::check(p)));
      return p.base;
    }

    inline base_t getBase(const pointer_t& p) {
      assert((IsA<PointerType::Heap>::check(p)));
      return p.base;
    }

    inline offset_t getOffset(const pointer_t& p) {
      assert((IsA<PointerType::Heap>::check(p)));
      return p.offset;
    }

    inline pointer_t ptradd(const pointer_t& ptr, offset_t off) {
      pointer_t ptr2;
      ptr2.segment = ptr.segment;
      ptr2.base = ptr.base;
      ptr2.offset = ptr.offset + off;
      return ptr2;
    }

    template <typename T>
    T CastPtr(const pointer_t& ptr) {
      return MiniMC::bit_cast<pointer_t, T>(ptr);
    }

    template <typename T>
    pointer_t CastToPtr(const T& val) {
      return MiniMC::bit_cast<T, pointer_t>(val);
    }

    inline pointer_t null_pointer() {
      pointer_t t;
      t.segment = 0;
      t.base = 0;
      t.offset = 0;
      return t;
    }
    
    inline bool is_null(const pointer_t& t) {
      return ::MiniMC::is_null(t);
    }

    using PtrBV = std::conditional<sizeof(pointer_t) == sizeof(MiniMC::BV64), MiniMC::BV64,
      std::conditional<sizeof(pointer_t) == sizeof(MiniMC::BV32),MiniMC::BV32,void>::type
      >::type;
    
  } // namespace Support
} // namespace MiniMC

#endif
