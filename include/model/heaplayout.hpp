#ifndef _HEAP_LAYOUT__
#define _HEAP_LAYOUT__

#include "support/pointer.hpp"
#include "host/types.hpp"

#include <vector>

namespace MiniMC {
  namespace Model {
    struct HeapBlock {
      MiniMC::pointer_t pointer;
      MiniMC::offset_t size;
    };

    class HeapLayout {
    public:
      pointer_t addBlock (MiniMC::offset_t size) {
	blocks.push_back ({MiniMC::Support::makeHeapPointer (blocks.size (),0),size});
	return blocks.back().pointer;
      }

      auto begin () const {
	return blocks.begin ();
      }

      auto end () const {
	return blocks.end ();
      }
      
    private:
      std::vector<HeapBlock> blocks;
    };
    
  }
}

#endif
